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
 * $Workfile: MediaServer_MicroStack.h
 * $Revision: #1.0.2384.30305
 * $Author:   Intel Corporation, Intel Device Builder
 * $Date:     Thursday, July 13, 2006
 *
 *
 *
 */
#ifndef __MediaServer_Microstack__
#define __MediaServer_Microstack__


#include "ILibAsyncSocket.h"

/*! \file MediaServer_MicroStack.h 
	\brief MicroStack APIs for Device Implementation
*/

/*! \defgroup MicroStack MicroStack Module
	\{
*/

struct MediaServer_DataObject;
struct packetheader;

typedef void* MediaServer_MicroStackToken;
typedef void* MediaServer_SessionToken;




/* Complex Type Parsers */


/* Complex Type Serializers */



/* MediaServer_ Stack Management */
MediaServer_MicroStackToken MediaServer_CreateMicroStack(void *Chain, const char* FriendlyName,const char* UDN, const char* SerialNumber, const int NotifyCycleSeconds, const unsigned short PortNum);


void MediaServer_IPAddressListChanged(MediaServer_MicroStackToken MicroStackToken);
int MediaServer_GetLocalPortNumber(MediaServer_SessionToken token);
int   MediaServer_GetLocalInterfaceToHost(const MediaServer_SessionToken MediaServer_Token);
void* MediaServer_GetWebServerToken(const MediaServer_MicroStackToken MicroStackToken);
void MediaServer_SetTag(const MediaServer_MicroStackToken token, void *UserToken);
void *MediaServer_GetTag(const MediaServer_MicroStackToken token);
MediaServer_MicroStackToken MediaServer_GetMicroStackTokenFromSessionToken(const MediaServer_SessionToken token);

typedef void(*MediaServer__ActionHandler_ConnectionManager_GetCurrentConnectionIDs) (void* upnptoken);
typedef void(*MediaServer__ActionHandler_ConnectionManager_GetCurrentConnectionInfo) (void* upnptoken,int ConnectionID);
typedef void(*MediaServer__ActionHandler_ConnectionManager_GetProtocolInfo) (void* upnptoken);
typedef void(*MediaServer__ActionHandler_ContentDirectory_Browse) (void* upnptoken,char* ObjectID,char* BrowseFlag,char* Filter,unsigned int StartingIndex,unsigned int RequestedCount,char* SortCriteria);
typedef void(*MediaServer__ActionHandler_ContentDirectory_CreateObject) (void* upnptoken,char* ContainerID,char* Elements);
typedef void(*MediaServer__ActionHandler_ContentDirectory_DestroyObject) (void* upnptoken,char* ObjectID);
typedef void(*MediaServer__ActionHandler_ContentDirectory_GetSearchCapabilities) (void* upnptoken);
typedef void(*MediaServer__ActionHandler_ContentDirectory_GetSortCapabilities) (void* upnptoken);
typedef void(*MediaServer__ActionHandler_ContentDirectory_GetSystemUpdateID) (void* upnptoken);
typedef void(*MediaServer__ActionHandler_ContentDirectory_Search) (void* upnptoken,char* ContainerID,char* SearchCriteria,char* Filter,unsigned int StartingIndex,unsigned int RequestedCount,char* SortCriteria);
/* MediaServer_ Set Function Pointers Methods */
extern void (*MediaServer_FP_PresentationPage) (void* upnptoken,struct packetheader *packet);
extern MediaServer__ActionHandler_ConnectionManager_GetCurrentConnectionIDs MediaServer_FP_ConnectionManager_GetCurrentConnectionIDs;
extern MediaServer__ActionHandler_ConnectionManager_GetCurrentConnectionInfo MediaServer_FP_ConnectionManager_GetCurrentConnectionInfo;
extern MediaServer__ActionHandler_ConnectionManager_GetProtocolInfo MediaServer_FP_ConnectionManager_GetProtocolInfo;
extern MediaServer__ActionHandler_ContentDirectory_Browse MediaServer_FP_ContentDirectory_Browse;
extern MediaServer__ActionHandler_ContentDirectory_CreateObject MediaServer_FP_ContentDirectory_CreateObject;
extern MediaServer__ActionHandler_ContentDirectory_DestroyObject MediaServer_FP_ContentDirectory_DestroyObject;
extern MediaServer__ActionHandler_ContentDirectory_GetSearchCapabilities MediaServer_FP_ContentDirectory_GetSearchCapabilities;
extern MediaServer__ActionHandler_ContentDirectory_GetSortCapabilities MediaServer_FP_ContentDirectory_GetSortCapabilities;
extern MediaServer__ActionHandler_ContentDirectory_GetSystemUpdateID MediaServer_FP_ContentDirectory_GetSystemUpdateID;
extern MediaServer__ActionHandler_ContentDirectory_Search MediaServer_FP_ContentDirectory_Search;


void MediaServer_SetDisconnectFlag(MediaServer_SessionToken token,void *flag);

/* Invocation Response Methods */
void MediaServer_Response_Error(const MediaServer_SessionToken MediaServer_Token, const int ErrorCode, const char* ErrorMsg);
void MediaServer_ResponseGeneric(const MediaServer_SessionToken MediaServer_Token,const char* ServiceURI,const char* MethodName,const char* Params);
int MediaServer_AsyncResponse_START(const MediaServer_SessionToken MediaServer_Token, const char* actionName, const char* serviceUrnWithVersion);
int MediaServer_AsyncResponse_DONE(const MediaServer_SessionToken MediaServer_Token, const char* actionName);
int MediaServer_AsyncResponse_OUT(const MediaServer_SessionToken MediaServer_Token, const char* outArgName, const char* bytes, const int byteLength, enum ILibAsyncSocket_MemoryOwnership bytesMemoryOwnership,const int startArg, const int endArg);
void MediaServer_Response_ConnectionManager_GetCurrentConnectionIDs(const MediaServer_SessionToken MediaServer_Token, const char* ConnectionIDs);
void MediaServer_Response_ConnectionManager_GetCurrentConnectionInfo(const MediaServer_SessionToken MediaServer_Token, const int RcsID, const int AVTransportID, const char* ProtocolInfo, const char* PeerConnectionManager, const int PeerConnectionID, const char* Direction, const char* Status);
void MediaServer_Response_ConnectionManager_GetProtocolInfo(const MediaServer_SessionToken MediaServer_Token, const char* Source, const char* Sink);
void MediaServer_Response_ContentDirectory_Browse(const MediaServer_SessionToken MediaServer_Token, const char* Result, const unsigned int NumberReturned, const unsigned int TotalMatches, const unsigned int UpdateID);
void MediaServer_Response_ContentDirectory_CreateObject(const MediaServer_SessionToken MediaServer_Token, const char* ObjectID, const char* Result);
void MediaServer_Response_ContentDirectory_DestroyObject(const MediaServer_SessionToken MediaServer_Token);
void MediaServer_Response_ContentDirectory_GetSearchCapabilities(const MediaServer_SessionToken MediaServer_Token, const char* SearchCaps);
void MediaServer_Response_ContentDirectory_GetSortCapabilities(const MediaServer_SessionToken MediaServer_Token, const char* SortCaps);
void MediaServer_Response_ContentDirectory_GetSystemUpdateID(const MediaServer_SessionToken MediaServer_Token, const unsigned int Id);
void MediaServer_Response_ContentDirectory_Search(const MediaServer_SessionToken MediaServer_Token, const char* Result, const unsigned int NumberReturned, const unsigned int TotalMatches, const unsigned int UpdateID);

/* The string parameters for the following response methods MUST be MANUALLY escaped */
/* void MediaServer_Response_ContentDirectory_Browse */
/* void MediaServer_Response_ContentDirectory_CreateObject */
/* void MediaServer_Response_ContentDirectory_Search */


/* State Variable Eventing Methods */
void MediaServer_SetState_ConnectionManager_SourceProtocolInfo(MediaServer_MicroStackToken microstack,char* val);
void MediaServer_SetState_ConnectionManager_SinkProtocolInfo(MediaServer_MicroStackToken microstack,char* val);
void MediaServer_SetState_ConnectionManager_CurrentConnectionIDs(MediaServer_MicroStackToken microstack,char* val);
void MediaServer_SetState_ContentDirectory_ContainerUpdateIDs(MediaServer_MicroStackToken microstack,char* val);
void MediaServer_SetState_ContentDirectory_SystemUpdateID(MediaServer_MicroStackToken microstack,unsigned int val);


#define MediaServer__StateVariable_AllowedValues_MAX 6
struct MediaServer__StateVariableTable_ConnectionManager
{
	char Reserved[347];
	int ReservedXL;
	int ReservedUXL;
};
struct MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_ProtocolInfo
{
	int Reserved1;
	int Reserved1L;
	int Reserved8;
	int Reserved8L;
};
struct MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_ConnectionStatus
{
	int Reserved1;
	int Reserved1L;
	int Reserved2;
	int Reserved2L;
	int Reserved3;
	int Reserved3L;
	char *AllowedValues[MediaServer__StateVariable_AllowedValues_MAX];
	int Reserved8;
	int Reserved8L;
};
struct MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_AVTransportID
{
	int Reserved1;
	int Reserved1L;
	int Reserved8;
	int Reserved8L;
};
struct MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_RcsID
{
	int Reserved1;
	int Reserved1L;
	int Reserved8;
	int Reserved8L;
};
struct MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_ConnectionID
{
	int Reserved1;
	int Reserved1L;
	int Reserved8;
	int Reserved8L;
};
struct MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_ConnectionManager
{
	int Reserved1;
	int Reserved1L;
	int Reserved8;
	int Reserved8L;
};
struct MediaServer__StateVariable_ConnectionManager_SourceProtocolInfo
{
	int Reserved1;
	int Reserved1L;
	int Reserved8;
	int Reserved8L;
};
struct MediaServer__StateVariable_ConnectionManager_SinkProtocolInfo
{
	int Reserved1;
	int Reserved1L;
	int Reserved8;
	int Reserved8L;
};
struct MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_Direction
{
	int Reserved1;
	int Reserved1L;
	int Reserved2;
	int Reserved2L;
	int Reserved3;
	int Reserved3L;
	char *AllowedValues[MediaServer__StateVariable_AllowedValues_MAX];
	int Reserved8;
	int Reserved8L;
};
struct MediaServer__StateVariable_ConnectionManager_CurrentConnectionIDs
{
	int Reserved1;
	int Reserved1L;
	int Reserved8;
	int Reserved8L;
};
struct MediaServer__StateVariableTable_ContentDirectory
{
	char Reserved[323];
	int ReservedXL;
	int ReservedUXL;
};
struct MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_BrowseFlag
{
	int Reserved1;
	int Reserved1L;
	int Reserved2;
	int Reserved2L;
	int Reserved3;
	int Reserved3L;
	char *AllowedValues[MediaServer__StateVariable_AllowedValues_MAX];
	int Reserved8;
	int Reserved8L;
};
struct MediaServer__StateVariable_ContentDirectory_ContainerUpdateIDs
{
	int Reserved1;
	int Reserved1L;
	int Reserved8;
	int Reserved8L;
};
struct MediaServer__StateVariable_ContentDirectory_SystemUpdateID
{
	int Reserved1;
	int Reserved1L;
	int Reserved8;
	int Reserved8L;
};
struct MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_SortCriteria
{
	int Reserved1;
	int Reserved1L;
	int Reserved8;
	int Reserved8L;
};
struct MediaServer__StateVariable_ContentDirectory_SortCapabilities
{
	int Reserved1;
	int Reserved1L;
	int Reserved8;
	int Reserved8L;
};
struct MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_UpdateID
{
	int Reserved1;
	int Reserved1L;
	int Reserved8;
	int Reserved8L;
};
struct MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_Index
{
	int Reserved1;
	int Reserved1L;
	int Reserved8;
	int Reserved8L;
};
struct MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_ObjectID
{
	int Reserved1;
	int Reserved1L;
	int Reserved8;
	int Reserved8L;
};
struct MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_Count
{
	int Reserved1;
	int Reserved1L;
	int Reserved8;
	int Reserved8L;
};
struct MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_Result
{
	int Reserved1;
	int Reserved1L;
	int Reserved8;
	int Reserved8L;
};
struct MediaServer__StateVariable_ContentDirectory_SearchCapabilities
{
	int Reserved1;
	int Reserved1L;
	int Reserved8;
	int Reserved8L;
};
struct MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_SearchCriteria
{
	int Reserved1;
	int Reserved1L;
	int Reserved8;
	int Reserved8L;
};
struct MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_Filter
{
	int Reserved1;
	int Reserved1L;
	int Reserved8;
	int Reserved8L;
};
struct MediaServer__ActionTable_ConnectionManager
{
	char Reserved[342];
	int ReservedXL;
	int ReservedUXL;
};
struct MediaServer__Action_ConnectionManager_GetCurrentConnectionIDs
{
	int Reserved;
	int Reserved2;
};
struct MediaServer__Action_ConnectionManager_GetCurrentConnectionInfo
{
	int Reserved;
	int Reserved2;
};
struct MediaServer__Action_ConnectionManager_GetProtocolInfo
{
	int Reserved;
	int Reserved2;
};
struct MediaServer__ActionTable_ContentDirectory
{
	char Reserved[709];
	int ReservedXL;
	int ReservedUXL;
};
struct MediaServer__Action_ContentDirectory_Browse
{
	int Reserved;
	int Reserved2;
};
struct MediaServer__Action_ContentDirectory_CreateObject
{
	int Reserved;
	int Reserved2;
};
struct MediaServer__Action_ContentDirectory_DestroyObject
{
	int Reserved;
	int Reserved2;
};
struct MediaServer__Action_ContentDirectory_GetSearchCapabilities
{
	int Reserved;
	int Reserved2;
};
struct MediaServer__Action_ContentDirectory_GetSortCapabilities
{
	int Reserved;
	int Reserved2;
};
struct MediaServer__Action_ContentDirectory_GetSystemUpdateID
{
	int Reserved;
	int Reserved2;
};
struct MediaServer__Action_ContentDirectory_Search
{
	int Reserved;
	int Reserved2;
};
struct MediaServer__Service_ConnectionManager
{
	struct MediaServer__Action_ConnectionManager_GetCurrentConnectionIDs *GetCurrentConnectionIDs;
	struct MediaServer__Action_ConnectionManager_GetCurrentConnectionInfo *GetCurrentConnectionInfo;
	struct MediaServer__Action_ConnectionManager_GetProtocolInfo *GetProtocolInfo;
	
	struct MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_ProtocolInfo *StateVar_A_ARG_TYPE_ProtocolInfo;
	struct MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_ConnectionStatus *StateVar_A_ARG_TYPE_ConnectionStatus;
	struct MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_AVTransportID *StateVar_A_ARG_TYPE_AVTransportID;
	struct MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_RcsID *StateVar_A_ARG_TYPE_RcsID;
	struct MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_ConnectionID *StateVar_A_ARG_TYPE_ConnectionID;
	struct MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_ConnectionManager *StateVar_A_ARG_TYPE_ConnectionManager;
	struct MediaServer__StateVariable_ConnectionManager_SourceProtocolInfo *StateVar_SourceProtocolInfo;
	struct MediaServer__StateVariable_ConnectionManager_SinkProtocolInfo *StateVar_SinkProtocolInfo;
	struct MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_Direction *StateVar_A_ARG_TYPE_Direction;
	struct MediaServer__StateVariable_ConnectionManager_CurrentConnectionIDs *StateVar_CurrentConnectionIDs;
	
	char Reserved[159];
	int ReservedXL;
	int ReservedUXL;
};
struct MediaServer__Service_ContentDirectory
{
	struct MediaServer__Action_ContentDirectory_Browse *Browse;
	struct MediaServer__Action_ContentDirectory_CreateObject *CreateObject;
	struct MediaServer__Action_ContentDirectory_DestroyObject *DestroyObject;
	struct MediaServer__Action_ContentDirectory_GetSearchCapabilities *GetSearchCapabilities;
	struct MediaServer__Action_ContentDirectory_GetSortCapabilities *GetSortCapabilities;
	struct MediaServer__Action_ContentDirectory_GetSystemUpdateID *GetSystemUpdateID;
	struct MediaServer__Action_ContentDirectory_Search *Search;
	
	struct MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_BrowseFlag *StateVar_A_ARG_TYPE_BrowseFlag;
	struct MediaServer__StateVariable_ContentDirectory_ContainerUpdateIDs *StateVar_ContainerUpdateIDs;
	struct MediaServer__StateVariable_ContentDirectory_SystemUpdateID *StateVar_SystemUpdateID;
	struct MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_SortCriteria *StateVar_A_ARG_TYPE_SortCriteria;
	struct MediaServer__StateVariable_ContentDirectory_SortCapabilities *StateVar_SortCapabilities;
	struct MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_UpdateID *StateVar_A_ARG_TYPE_UpdateID;
	struct MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_Index *StateVar_A_ARG_TYPE_Index;
	struct MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_ObjectID *StateVar_A_ARG_TYPE_ObjectID;
	struct MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_Count *StateVar_A_ARG_TYPE_Count;
	struct MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_Result *StateVar_A_ARG_TYPE_Result;
	struct MediaServer__StateVariable_ContentDirectory_SearchCapabilities *StateVar_SearchCapabilities;
	struct MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_SearchCriteria *StateVar_A_ARG_TYPE_SearchCriteria;
	struct MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_Filter *StateVar_A_ARG_TYPE_Filter;
	
	char Reserved[158];
	int ReservedXL;
	int ReservedUXL;
};
struct MediaServer__Device_MediaServer
{
	struct MediaServer__Service_ConnectionManager *ConnectionManager;
	struct MediaServer__Service_ContentDirectory *ContentDirectory;
	
	const char *FriendlyName;
	const char *UDN;
	const char *Serial;
	const char *Manufacturer;
	const char *ManufacturerURL;
	const char *ModelDescription;
	const char *ModelName;
	const char *ModelNumber;
	const char *ModelURL;
	const char *ProductCode;
	char Reserved[467];
	int ReservedXL;
	int ReservedUXL;
};

struct MediaServer__Device_MediaServer* MediaServer_GetConfiguration();



/*! \} */
#endif
