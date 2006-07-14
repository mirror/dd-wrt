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
* $Workfile: MediaServer_MicroStack.c
* $Revision: #1.0.2384.30305
* $Author:   Intel Corporation, Intel Device Builder
* $Date:     Thursday, July 13, 2006
*
*
*
*/


#if defined(WIN32) || defined(_WIN32_WCE)
#	ifndef MICROSTACK_NO_STDAFX
#		include "stdafx.h"
#	endif
char* MediaServer_PLATFORM = "WINDOWS";
#else
char* MediaServer_PLATFORM = "POSIX";
#endif

#if defined(WIN32)
#define _CRTDBG_MAP_ALLOC
#endif

#if defined(WINSOCK2)
#	include <winsock2.h>
#	include <ws2tcpip.h>
#elif defined(WINSOCK1)
#	include <winsock.h>
#	include <wininet.h>
#endif

#include "ILibParsers.h"
#include "MediaServer_MicroStack.h"
#include "ILibWebServer.h"
#include "ILibWebClient.h"
#include "ILibAsyncSocket.h"
#include "ILibAsyncUDPSocket.h"

#if defined(WIN32) && !defined(_WIN32_WCE)
#include <crtdbg.h>
#endif

#define UPNP_SSDP_TTL 4
#define UPNP_HTTP_MAXSOCKETS 5
#define UPNP_MAX_SSDP_HEADER_SIZE 4096
#define UPNP_PORT 1900

#define UPNP_GROUP "239.255.255.250"
#define MediaServer__MAX_SUBSCRIPTION_TIMEOUT 300
#define MediaServer_MIN(a,b) (((a)<(b))?(a):(b))

#define LVL3DEBUG(x)

struct MediaServer__StateVariableTable_ConnectionManager MediaServer__StateVariableTable_ConnectionManager_Impl = 
{
   {
      (char)0x3E,(char)0x3C,(char)0x73,(char)0x74,(char)0x61,(char)0x74,(char)0x65,(char)0x56,(char)0x61,(char)0x72
      ,(char)0x69,(char)0x61,(char)0x62,(char)0x6C,(char)0x65,(char)0x20,(char)0x73,(char)0x65,(char)0x6E,(char)0x64
      ,(char)0x45,(char)0x76,(char)0x65,(char)0x6E,(char)0x74,(char)0x73,(char)0x3D,(char)0x22,(char)0x6E,(char)0x6F
      ,(char)0x22,(char)0x3E,(char)0x3C,(char)0x6E,(char)0x61,(char)0x6D,(char)0x65,(char)0x3E,(char)0x41,(char)0x5F
      ,(char)0x41,(char)0x52,(char)0x47,(char)0x5F,(char)0x54,(char)0x59,(char)0x50,(char)0x45,(char)0x5F,(char)0x50
      ,(char)0x72,(char)0x6F,(char)0x74,(char)0x6F,(char)0x63,(char)0x6F,(char)0x6C,(char)0x49,(char)0x6E,(char)0x66
      ,(char)0x6F,(char)0x3C,(char)0x2F,(char)0x85,(char)0x07,(char)0x12,(char)0x3C,(char)0x64,(char)0x61,(char)0x74
      ,(char)0x61,(char)0x54,(char)0x79,(char)0x70,(char)0x65,(char)0x3E,(char)0x73,(char)0x74,(char)0x72,(char)0x69
      ,(char)0x6E,(char)0x67,(char)0x3C,(char)0x2F,(char)0x49,(char)0x04,(char)0x02,(char)0x3C,(char)0x2F,(char)0xCD
      ,(char)0x17,(char)0x01,(char)0x3E,(char)0xB0,(char)0x1B,(char)0x10,(char)0x43,(char)0x6F,(char)0x6E,(char)0x6E
      ,(char)0x65,(char)0x63,(char)0x74,(char)0x69,(char)0x6F,(char)0x6E,(char)0x53,(char)0x74,(char)0x61,(char)0x74
      ,(char)0x75,(char)0x73,(char)0xA3,(char)0x1C,(char)0x10,(char)0x61,(char)0x6C,(char)0x6C,(char)0x6F,(char)0x77
      ,(char)0x65,(char)0x64,(char)0x56,(char)0x61,(char)0x6C,(char)0x75,(char)0x65,(char)0x4C,(char)0x69,(char)0x73
      ,(char)0x74,(char)0x8E,(char)0x04,(char)0x05,(char)0x3E,(char)0x4F,(char)0x4B,(char)0x3C,(char)0x2F,(char)0x4D
      ,(char)0x04,(char)0x00,(char)0xCE,(char)0x07,(char)0x15,(char)0x43,(char)0x6F,(char)0x6E,(char)0x74,(char)0x65
      ,(char)0x6E,(char)0x74,(char)0x46,(char)0x6F,(char)0x72,(char)0x6D,(char)0x61,(char)0x74,(char)0x4D,(char)0x69
      ,(char)0x73,(char)0x6D,(char)0x61,(char)0x74,(char)0x63,(char)0x68,(char)0x9D,(char)0x0C,(char)0x14,(char)0x49
      ,(char)0x6E,(char)0x73,(char)0x75,(char)0x66,(char)0x66,(char)0x69,(char)0x63,(char)0x69,(char)0x65,(char)0x6E
      ,(char)0x74,(char)0x42,(char)0x61,(char)0x6E,(char)0x64,(char)0x77,(char)0x69,(char)0x64,(char)0x74,(char)0x9E
      ,(char)0x0C,(char)0x05,(char)0x55,(char)0x6E,(char)0x72,(char)0x65,(char)0x6C,(char)0xC5,(char)0x5B,(char)0x07
      ,(char)0x43,(char)0x68,(char)0x61,(char)0x6E,(char)0x6E,(char)0x65,(char)0x6C,(char)0x9F,(char)0x0B,(char)0x05
      ,(char)0x6B,(char)0x6E,(char)0x6F,(char)0x77,(char)0x6E,(char)0x90,(char)0x2D,(char)0x01,(char)0x2F,(char)0x12
      ,(char)0x3A,(char)0x00,(char)0x3F,(char)0x5B,(char)0x0D,(char)0x41,(char)0x56,(char)0x54,(char)0x72,(char)0x61
      ,(char)0x6E,(char)0x73,(char)0x70,(char)0x6F,(char)0x72,(char)0x74,(char)0x49,(char)0x44,(char)0xD1,(char)0x76
      ,(char)0x02,(char)0x69,(char)0x34,(char)0xFF,(char)0x75,(char)0x00,(char)0x4C,(char)0x91,(char)0x03,(char)0x52
      ,(char)0x63,(char)0x73,(char)0xFF,(char)0x18,(char)0x00,(char)0xAB,(char)0x8E,(char)0x00,(char)0xBF,(char)0x1A
      ,(char)0x00,(char)0x2B,(char)0xA9,(char)0x07,(char)0x4D,(char)0x61,(char)0x6E,(char)0x61,(char)0x67,(char)0x65
      ,(char)0x72,(char)0xFF,(char)0xC5,(char)0x00,(char)0x4E,(char)0xE1,(char)0x03,(char)0x79,(char)0x65,(char)0x73
      ,(char)0x88,(char)0xE1,(char)0x06,(char)0x53,(char)0x6F,(char)0x75,(char)0x72,(char)0x63,(char)0x65,(char)0x7F
      ,(char)0xE0,(char)0x00,(char)0xA6,(char)0x1A,(char)0x03,(char)0x69,(char)0x6E,(char)0x6B,(char)0x7F,(char)0xFA
      ,(char)0x00,(char)0x6F,(char)0xFA,(char)0x03,(char)0x44,(char)0x69,(char)0x72,(char)0x06,(char)0xFA,(char)0x00
      ,(char)0xBF,(char)0xF8,(char)0x00,(char)0x45,(char)0xE4,(char)0x03,(char)0x70,(char)0x75,(char)0x74,(char)0x5D
      ,(char)0xF9,(char)0x06,(char)0x4F,(char)0x75,(char)0x74,(char)0x70,(char)0x75,(char)0x74,(char)0xBF,(char)0xD4
      ,(char)0x00,(char)0xD9,(char)0x69,(char)0x07,(char)0x43,(char)0x75,(char)0x72,(char)0x72,(char)0x65,(char)0x6E
      ,(char)0x74,(char)0x4C,(char)0xA0,(char)0x01,(char)0x73,(char)0xF2,(char)0x84
   },
   347,
   1432
};
struct MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_ProtocolInfo MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_ProtocolInfo_Impl = 
{
   0,
   94,
   94,
   16
};
struct MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_ConnectionStatus MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_ConnectionStatus_Impl = 
{
   110,
   98,
   208,
   18,
   439,
   19,
   {"OK","ContentFormatMismatch","InsufficientBandwidth","UnreliableChannel","Unknown",NULL},
   458,
   16
};
struct MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_AVTransportID MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_AVTransportID_Impl = 
{
   474,
   91,
   565,
   16
};
struct MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_RcsID MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_RcsID_Impl = 
{
   581,
   83,
   664,
   16
};
struct MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_ConnectionID MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_ConnectionID_Impl = 
{
   680,
   90,
   770,
   16
};
struct MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_ConnectionManager MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_ConnectionManager_Impl = 
{
   786,
   99,
   885,
   16
};
struct MediaServer__StateVariable_ConnectionManager_SourceProtocolInfo MediaServer__StateVariable_ConnectionManager_SourceProtocolInfo_Impl = 
{
   901,
   90,
   991,
   16
};
struct MediaServer__StateVariable_ConnectionManager_SinkProtocolInfo MediaServer__StateVariable_ConnectionManager_SinkProtocolInfo_Impl = 
{
   1007,
   88,
   1095,
   16
};
struct MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_Direction MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_Direction_Impl = 
{
   1111,
   91,
   1202,
   18,
   1289,
   19,
   {"Input","Output",NULL},
   1308,
   16
};
struct MediaServer__StateVariable_ConnectionManager_CurrentConnectionIDs MediaServer__StateVariable_ConnectionManager_CurrentConnectionIDs_Impl = 
{
   1324,
   92,
   1416,
   16
};
struct MediaServer__StateVariableTable_ContentDirectory MediaServer__StateVariableTable_ContentDirectory_Impl = 
{
   {
      (char)0x3C,(char)0x3C,(char)0x73,(char)0x74,(char)0x61,(char)0x74,(char)0x65,(char)0x56,(char)0x61,(char)0x72
      ,(char)0x69,(char)0x61,(char)0x62,(char)0x6C,(char)0x65,(char)0x20,(char)0x73,(char)0x65,(char)0x6E,(char)0x64
      ,(char)0x45,(char)0x76,(char)0x65,(char)0x6E,(char)0x74,(char)0x73,(char)0x3D,(char)0x22,(char)0x6E,(char)0x6F
      ,(char)0x22,(char)0x3E,(char)0x3C,(char)0x6E,(char)0x61,(char)0x6D,(char)0x65,(char)0x3E,(char)0x41,(char)0x5F
      ,(char)0x41,(char)0x52,(char)0x47,(char)0x5F,(char)0x54,(char)0x59,(char)0x50,(char)0x45,(char)0x5F,(char)0x42
      ,(char)0x72,(char)0x6F,(char)0x77,(char)0x73,(char)0x65,(char)0x46,(char)0x6C,(char)0x61,(char)0x67,(char)0x3C
      ,(char)0x2F,(char)0x05,(char)0x07,(char)0x12,(char)0x3C,(char)0x64,(char)0x61,(char)0x74,(char)0x61,(char)0x54
      ,(char)0x79,(char)0x70,(char)0x65,(char)0x3E,(char)0x73,(char)0x74,(char)0x72,(char)0x69,(char)0x6E,(char)0x67
      ,(char)0x3C,(char)0x2F,(char)0x49,(char)0x04,(char)0x11,(char)0x3C,(char)0x61,(char)0x6C,(char)0x6C,(char)0x6F
      ,(char)0x77,(char)0x65,(char)0x64,(char)0x56,(char)0x61,(char)0x6C,(char)0x75,(char)0x65,(char)0x4C,(char)0x69
      ,(char)0x73,(char)0x74,(char)0x8E,(char)0x04,(char)0x01,(char)0x3E,(char)0x06,(char)0x13,(char)0x04,(char)0x4D
      ,(char)0x65,(char)0x74,(char)0x61,(char)0x04,(char)0x11,(char)0x02,(char)0x3C,(char)0x2F,(char)0x4D,(char)0x07
      ,(char)0x00,(char)0xD4,(char)0x0A,(char)0x0E,(char)0x44,(char)0x69,(char)0x72,(char)0x65,(char)0x63,(char)0x74
      ,(char)0x43,(char)0x68,(char)0x69,(char)0x6C,(char)0x64,(char)0x72,(char)0x65,(char)0x6E,(char)0x50,(char)0x0C
      ,(char)0x01,(char)0x2F,(char)0xD2,(char)0x1B,(char)0x01,(char)0x2F,(char)0x8D,(char)0x37,(char)0x01,(char)0x3E
      ,(char)0x5B,(char)0x3B,(char)0x03,(char)0x79,(char)0x65,(char)0x73,(char)0x88,(char)0x3B,(char)0x12,(char)0x43
      ,(char)0x6F,(char)0x6E,(char)0x74,(char)0x61,(char)0x69,(char)0x6E,(char)0x65,(char)0x72,(char)0x55,(char)0x70
      ,(char)0x64,(char)0x61,(char)0x74,(char)0x65,(char)0x49,(char)0x44,(char)0x73,(char)0xE3,(char)0x3A,(char)0x00
      ,(char)0xB5,(char)0x1A,(char)0x06,(char)0x53,(char)0x79,(char)0x73,(char)0x74,(char)0x65,(char)0x6D,(char)0xC8
      ,(char)0x19,(char)0x00,(char)0x51,(char)0x54,(char)0x03,(char)0x75,(char)0x69,(char)0x34,(char)0xF6,(char)0x18
      ,(char)0x00,(char)0x95,(char)0x6E,(char)0x0C,(char)0x53,(char)0x6F,(char)0x72,(char)0x74,(char)0x43,(char)0x72
      ,(char)0x69,(char)0x74,(char)0x65,(char)0x72,(char)0x69,(char)0x61,(char)0x7F,(char)0x34,(char)0x00,(char)0x18
      ,(char)0x8A,(char)0x00,(char)0xC5,(char)0x18,(char)0x0A,(char)0x61,(char)0x70,(char)0x61,(char)0x62,(char)0x69
      ,(char)0x6C,(char)0x69,(char)0x74,(char)0x69,(char)0x65,(char)0x3F,(char)0x4E,(char)0x00,(char)0xE4,(char)0xA3
      ,(char)0x00,(char)0x3F,(char)0x4F,(char)0x00,(char)0xA8,(char)0xBD,(char)0x05,(char)0x49,(char)0x6E,(char)0x64
      ,(char)0x65,(char)0x78,(char)0x3F,(char)0x68,(char)0x00,(char)0xA0,(char)0xD6,(char)0x08,(char)0x4F,(char)0x62
      ,(char)0x6A,(char)0x65,(char)0x63,(char)0x74,(char)0x49,(char)0x44,(char)0x7F,(char)0x4D,(char)0x00,(char)0x23
      ,(char)0xF1,(char)0x05,(char)0x43,(char)0x6F,(char)0x75,(char)0x6E,(char)0x74,(char)0xBF,(char)0x9B,(char)0x00
      ,(char)0xA0,(char)0x9B,(char)0x06,(char)0x52,(char)0x65,(char)0x73,(char)0x75,(char)0x6C,(char)0x74,(char)0x3F
      ,(char)0x9A,(char)0x00,(char)0x19,(char)0x9A,(char)0x05,(char)0x65,(char)0x61,(char)0x72,(char)0x63,(char)0x68
      ,(char)0xBF,(char)0x9A,(char)0x00,(char)0xF0,(char)0xCF,(char)0x00,(char)0x06,(char)0x1D,(char)0x00,(char)0x7F
      ,(char)0xD0,(char)0x00,(char)0xEA,(char)0xEB,(char)0x06,(char)0x46,(char)0x69,(char)0x6C,(char)0x74,(char)0x65
      ,(char)0x72,(char)0x72,(char)0xEA
   },
   323,
   1489
};
struct MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_BrowseFlag MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_BrowseFlag_Impl = 
{
   0,
   92,
   92,
   18,
   202,
   19,
   {"BrowseMetadata","BrowseDirectChildren",NULL},
   221,
   16
};
struct MediaServer__StateVariable_ContentDirectory_ContainerUpdateIDs MediaServer__StateVariable_ContentDirectory_ContainerUpdateIDs_Impl = 
{
   237,
   90,
   327,
   16
};
struct MediaServer__StateVariable_ContentDirectory_SystemUpdateID MediaServer__StateVariable_ContentDirectory_SystemUpdateID_Impl = 
{
   343,
   83,
   426,
   16
};
struct MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_SortCriteria MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_SortCriteria_Impl = 
{
   442,
   94,
   536,
   16
};
struct MediaServer__StateVariable_ContentDirectory_SortCapabilities MediaServer__StateVariable_ContentDirectory_SortCapabilities_Impl = 
{
   552,
   87,
   639,
   16
};
struct MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_UpdateID MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_UpdateID_Impl = 
{
   655,
   87,
   742,
   16
};
struct MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_Index MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_Index_Impl = 
{
   758,
   84,
   842,
   16
};
struct MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_ObjectID MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_ObjectID_Impl = 
{
   858,
   90,
   948,
   16
};
struct MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_Count MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_Count_Impl = 
{
   964,
   84,
   1048,
   16
};
struct MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_Result MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_Result_Impl = 
{
   1064,
   88,
   1152,
   16
};
struct MediaServer__StateVariable_ContentDirectory_SearchCapabilities MediaServer__StateVariable_ContentDirectory_SearchCapabilities_Impl = 
{
   1168,
   89,
   1257,
   16
};
struct MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_SearchCriteria MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_SearchCriteria_Impl = 
{
   1273,
   96,
   1369,
   16
};
struct MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_Filter MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_Filter_Impl = 
{
   1385,
   88,
   1473,
   16
};
struct MediaServer__ActionTable_ConnectionManager MediaServer__ActionTable_ConnectionManager_Impl = 
{
   {
      (char)0x1D,(char)0x3C,(char)0x61,(char)0x63,(char)0x74,(char)0x69,(char)0x6F,(char)0x6E,(char)0x3E,(char)0x3C
      ,(char)0x6E,(char)0x61,(char)0x6D,(char)0x65,(char)0x3E,(char)0x47,(char)0x65,(char)0x74,(char)0x43,(char)0x75
      ,(char)0x72,(char)0x72,(char)0x65,(char)0x6E,(char)0x74,(char)0x43,(char)0x6F,(char)0x6E,(char)0x6E,(char)0x65
      ,(char)0xC5,(char)0x06,(char)0x05,(char)0x49,(char)0x44,(char)0x73,(char)0x3C,(char)0x2F,(char)0x85,(char)0x07
      ,(char)0x0D,(char)0x3C,(char)0x61,(char)0x72,(char)0x67,(char)0x75,(char)0x6D,(char)0x65,(char)0x6E,(char)0x74
      ,(char)0x4C,(char)0x69,(char)0x73,(char)0x74,(char)0x8A,(char)0x03,(char)0x00,(char)0x07,(char)0x0F,(char)0x00
      ,(char)0x95,(char)0x0C,(char)0x03,(char)0x64,(char)0x69,(char)0x72,(char)0x86,(char)0x11,(char)0x06,(char)0x3E
      ,(char)0x6F,(char)0x75,(char)0x74,(char)0x3C,(char)0x2F,(char)0xCA,(char)0x03,(char)0x16,(char)0x3C,(char)0x72
      ,(char)0x65,(char)0x6C,(char)0x61,(char)0x74,(char)0x65,(char)0x64,(char)0x53,(char)0x74,(char)0x61,(char)0x74
      ,(char)0x65,(char)0x56,(char)0x61,(char)0x72,(char)0x69,(char)0x61,(char)0x62,(char)0x6C,(char)0x65,(char)0x3E
      ,(char)0x56,(char)0x1F,(char)0x00,(char)0xD5,(char)0x0A,(char)0x02,(char)0x3C,(char)0x2F,(char)0x0A,(char)0x20
      ,(char)0x01,(char)0x2F,(char)0x4E,(char)0x26,(char)0x01,(char)0x2F,(char)0x08,(char)0x35,(char)0x00,(char)0x22
      ,(char)0x37,(char)0x03,(char)0x6E,(char)0x66,(char)0x6F,(char)0x71,(char)0x37,(char)0x00,(char)0x12,(char)0x37
      ,(char)0x02,(char)0x69,(char)0x6E,(char)0xE2,(char)0x36,(char)0x0B,(char)0x41,(char)0x5F,(char)0x41,(char)0x52
      ,(char)0x47,(char)0x5F,(char)0x54,(char)0x59,(char)0x50,(char)0x45,(char)0x5F,(char)0x4E,(char)0x13,(char)0x00
      ,(char)0xA1,(char)0x37,(char)0x00,(char)0x0F,(char)0x5A,(char)0x05,(char)0x52,(char)0x63,(char)0x73,(char)0x49
      ,(char)0x44,(char)0x37,(char)0x58,(char)0x00,(char)0x4B,(char)0x21,(char)0x03,(char)0x52,(char)0x63,(char)0x73
      ,(char)0xB4,(char)0x1F,(char)0x0B,(char)0x41,(char)0x56,(char)0x54,(char)0x72,(char)0x61,(char)0x6E,(char)0x73
      ,(char)0x70,(char)0x6F,(char)0x72,(char)0x74,(char)0xBF,(char)0x21,(char)0x00,(char)0xC5,(char)0x42,(char)0x00
      ,(char)0xCF,(char)0x13,(char)0x00,(char)0x30,(char)0x43,(char)0x08,(char)0x50,(char)0x72,(char)0x6F,(char)0x74
      ,(char)0x6F,(char)0x63,(char)0x6F,(char)0x6C,(char)0x0C,(char)0x72,(char)0x00,(char)0xFA,(char)0x44,(char)0x00
      ,(char)0x8E,(char)0x13,(char)0x00,(char)0x31,(char)0x23,(char)0x03,(char)0x65,(char)0x65,(char)0x72,(char)0x8A
      ,(char)0xCD,(char)0x07,(char)0x4D,(char)0x61,(char)0x6E,(char)0x61,(char)0x67,(char)0x65,(char)0x72,(char)0x3F
      ,(char)0x6A,(char)0x03,(char)0x50,(char)0x45,(char)0x5F,(char)0xD3,(char)0x14,(char)0x00,(char)0xBE,(char)0x26
      ,(char)0x00,(char)0x7F,(char)0x8F,(char)0x00,(char)0xBF,(char)0xB0,(char)0x00,(char)0x84,(char)0xE2,(char)0x01
      ,(char)0x44,(char)0x48,(char)0xCE,(char)0x00,(char)0xBF,(char)0xB1,(char)0x03,(char)0x50,(char)0x45,(char)0x5F
      ,(char)0xCB,(char)0x12,(char)0x00,(char)0x30,(char)0xD2,(char)0x00,(char)0xC4,(char)0xE7,(char)0x02,(char)0x75
      ,(char)0x73,(char)0x7F,(char)0x68,(char)0x00,(char)0x8D,(char)0xF3,(char)0x00,(char)0x88,(char)0x14,(char)0x00
      ,(char)0xA1,(char)0xF4,(char)0x00,(char)0x49,(char)0xF7,(char)0x03,(char)0x4C,(char)0x69,(char)0x73,(char)0xC5
      ,(char)0x03,(char)0x00,(char)0x07,(char)0xED,(char)0x00,(char)0x08,(char)0x02,(char)0x00,(char)0x05,(char)0xFA
      ,(char)0x03,(char)0x47,(char)0x65,(char)0x74,(char)0xD4,(char)0xB7,(char)0x00,(char)0xCE,(char)0x0E,(char)0x00
      ,(char)0x10,(char)0x35,(char)0x05,(char)0x6F,(char)0x75,(char)0x72,(char)0x63,(char)0x65,(char)0xF7,(char)0xE5
      ,(char)0x00,(char)0x46,(char)0x0F,(char)0x00,(char)0x7E,(char)0xC1,(char)0x04,(char)0x53,(char)0x69,(char)0x6E
      ,(char)0x6B,(char)0xF8,(char)0x1F,(char)0x03,(char)0x69,(char)0x6E,(char)0x6B,(char)0xAF,(char)0xE0,(char)0x00
      ,(char)0x17,(char)0x52
   },
   342,
   1748
};
struct MediaServer__Action_ConnectionManager_GetCurrentConnectionIDs MediaServer__Action_ConnectionManager_GetCurrentConnectionIDs_Impl = 
{
   0,
   220
};
struct MediaServer__Action_ConnectionManager_GetCurrentConnectionInfo MediaServer__Action_ConnectionManager_GetCurrentConnectionInfo_Impl = 
{
   220,
   1200
};
struct MediaServer__Action_ConnectionManager_GetProtocolInfo MediaServer__Action_ConnectionManager_GetProtocolInfo_Impl = 
{
   1420,
   328
};
struct MediaServer__ActionTable_ContentDirectory MediaServer__ActionTable_ContentDirectory_Impl = 
{
   {
      (char)0x16,(char)0x3C,(char)0x61,(char)0x63,(char)0x74,(char)0x69,(char)0x6F,(char)0x6E,(char)0x3E,(char)0x3C
      ,(char)0x6E,(char)0x61,(char)0x6D,(char)0x65,(char)0x3E,(char)0x42,(char)0x72,(char)0x6F,(char)0x77,(char)0x73
      ,(char)0x65,(char)0x3C,(char)0x2F,(char)0x45,(char)0x03,(char)0x0D,(char)0x3C,(char)0x61,(char)0x72,(char)0x67
      ,(char)0x75,(char)0x6D,(char)0x65,(char)0x6E,(char)0x74,(char)0x4C,(char)0x69,(char)0x73,(char)0x74,(char)0x8A
      ,(char)0x03,(char)0x00,(char)0xC7,(char)0x0A,(char)0x08,(char)0x4F,(char)0x62,(char)0x6A,(char)0x65,(char)0x63
      ,(char)0x74,(char)0x49,(char)0x44,(char)0x48,(char)0x0B,(char)0x04,(char)0x64,(char)0x69,(char)0x72,(char)0x65
      ,(char)0xC6,(char)0x12,(char)0x04,(char)0x69,(char)0x6E,(char)0x3C,(char)0x2F,(char)0x8A,(char)0x03,(char)0x21
      ,(char)0x3C,(char)0x72,(char)0x65,(char)0x6C,(char)0x61,(char)0x74,(char)0x65,(char)0x64,(char)0x53,(char)0x74
      ,(char)0x61,(char)0x74,(char)0x65,(char)0x56,(char)0x61,(char)0x72,(char)0x69,(char)0x61,(char)0x62,(char)0x6C
      ,(char)0x65,(char)0x3E,(char)0x41,(char)0x5F,(char)0x41,(char)0x52,(char)0x47,(char)0x5F,(char)0x54,(char)0x59
      ,(char)0x50,(char)0x45,(char)0x5F,(char)0x4A,(char)0x12,(char)0x00,(char)0x95,(char)0x0A,(char)0x02,(char)0x3C
      ,(char)0x2F,(char)0x4A,(char)0x1E,(char)0x00,(char)0xCF,(char)0x20,(char)0x00,(char)0x86,(char)0x2B,(char)0x04
      ,(char)0x46,(char)0x6C,(char)0x61,(char)0x67,(char)0x7F,(char)0x21,(char)0x02,(char)0x45,(char)0x5F,(char)0xCC
      ,(char)0x12,(char)0x00,(char)0xF0,(char)0x21,(char)0x06,(char)0x46,(char)0x69,(char)0x6C,(char)0x74,(char)0x65
      ,(char)0x72,(char)0x3F,(char)0x42,(char)0x02,(char)0x45,(char)0x5F,(char)0xC8,(char)0x11,(char)0x00,(char)0xB0
      ,(char)0x41,(char)0x0D,(char)0x53,(char)0x74,(char)0x61,(char)0x72,(char)0x74,(char)0x69,(char)0x6E,(char)0x67
      ,(char)0x49,(char)0x6E,(char)0x64,(char)0x65,(char)0x78,(char)0xBF,(char)0x63,(char)0x02,(char)0x45,(char)0x5F
      ,(char)0x87,(char)0x11,(char)0x00,(char)0xF0,(char)0x62,(char)0x0E,(char)0x52,(char)0x65,(char)0x71,(char)0x75
      ,(char)0x65,(char)0x73,(char)0x74,(char)0x65,(char)0x64,(char)0x43,(char)0x6F,(char)0x75,(char)0x6E,(char)0x74
      ,(char)0x3F,(char)0x85,(char)0x02,(char)0x45,(char)0x5F,(char)0x87,(char)0x11,(char)0x00,(char)0xF1,(char)0x42
      ,(char)0x0B,(char)0x6F,(char)0x72,(char)0x74,(char)0x43,(char)0x72,(char)0x69,(char)0x74,(char)0x65,(char)0x72
      ,(char)0x69,(char)0x61,(char)0x3F,(char)0xA6,(char)0x02,(char)0x45,(char)0x5F,(char)0x4E,(char)0x13,(char)0x00
      ,(char)0x72,(char)0x44,(char)0x03,(char)0x73,(char)0x75,(char)0x6C,(char)0x53,(char)0x42,(char)0x03,(char)0x6F
      ,(char)0x75,(char)0x74,(char)0xAD,(char)0xC7,(char)0x00,(char)0x08,(char)0x12,(char)0x00,(char)0x30,(char)0xC7
      ,(char)0x0E,(char)0x4E,(char)0x75,(char)0x6D,(char)0x62,(char)0x65,(char)0x72,(char)0x52,(char)0x65,(char)0x74
      ,(char)0x75,(char)0x72,(char)0x6E,(char)0x65,(char)0x64,(char)0x3F,(char)0x22,(char)0x00,(char)0xBA,(char)0x64
      ,(char)0x0C,(char)0x54,(char)0x6F,(char)0x74,(char)0x61,(char)0x6C,(char)0x4D,(char)0x61,(char)0x74,(char)0x63
      ,(char)0x68,(char)0x65,(char)0x73,(char)0x7F,(char)0x21,(char)0x00,(char)0xFA,(char)0x85,(char)0x08,(char)0x55
      ,(char)0x70,(char)0x64,(char)0x61,(char)0x74,(char)0x65,(char)0x49,(char)0x44,(char)0xBF,(char)0x63,(char)0x03
      ,(char)0x50,(char)0x45,(char)0x5F,(char)0x8A,(char)0x12,(char)0x00,(char)0xA1,(char)0xE9,(char)0x00,(char)0x49
      ,(char)0xEC,(char)0x03,(char)0x4C,(char)0x69,(char)0x73,(char)0xC5,(char)0x03,(char)0x00,(char)0x47,(char)0xE0
      ,(char)0x00,(char)0x08,(char)0x02,(char)0x00,(char)0x05,(char)0xEF,(char)0x0B,(char)0x43,(char)0x72,(char)0x65
      ,(char)0x61,(char)0x74,(char)0x65,(char)0x4F,(char)0x62,(char)0x6A,(char)0x65,(char)0x63,(char)0x49,(char)0xCD
      ,(char)0x00,(char)0x0E,(char)0x0E,(char)0x00,(char)0x4F,(char)0xFB,(char)0x0B,(char)0x43,(char)0x6F,(char)0x6E
      ,(char)0x74,(char)0x61,(char)0x69,(char)0x6E,(char)0x65,(char)0x72,(char)0x49,(char)0x44,(char)0xFF,(char)0xFA
      ,(char)0x02,(char)0x45,(char)0x5F,(char)0xC6,(char)0x1D,(char)0x02,(char)0x49,(char)0x44,(char)0xB2,(char)0xFB
      ,(char)0x03,(char)0x45,(char)0x6C,(char)0x65,(char)0x04,(char)0xFF,(char)0x01,(char)0x73,(char)0x3F,(char)0xFA
      ,(char)0x00,(char)0xBA,(char)0xB7,(char)0x00,(char)0xCA,(char)0x2E,(char)0x00,(char)0x3F,(char)0xD8,(char)0x00
      ,(char)0x7B,(char)0x41,(char)0x00,(char)0xBF,(char)0xF8,(char)0x00,(char)0xB2,(char)0xF8,(char)0x00,(char)0xA5
      ,(char)0x94,(char)0x07,(char)0x44,(char)0x65,(char)0x73,(char)0x74,(char)0x72,(char)0x6F,(char)0x79,(char)0xEB
      ,(char)0x94,(char)0x00,(char)0x1A,(char)0x53,(char)0x00,(char)0x3F,(char)0x94,(char)0x00,(char)0x7F,(char)0xC7
      ,(char)0x16,(char)0x3E,(char)0x47,(char)0x65,(char)0x74,(char)0x53,(char)0x65,(char)0x61,(char)0x72,(char)0x63
      ,(char)0x68,(char)0x43,(char)0x61,(char)0x70,(char)0x61,(char)0x62,(char)0x69,(char)0x6C,(char)0x69,(char)0x74
      ,(char)0x69,(char)0x65,(char)0x73,(char)0xA5,(char)0xC9,(char)0x00,(char)0xC9,(char)0x0D,(char)0x01,(char)0x73
      ,(char)0xF7,(char)0xFC,(char)0x00,(char)0x14,(char)0x1E,(char)0x00,(char)0x7F,(char)0x35,(char)0x00,(char)0x4B
      ,(char)0x35,(char)0x03,(char)0x6F,(char)0x72,(char)0x74,(char)0xF2,(char)0x34,(char)0x03,(char)0x6F,(char)0x72
      ,(char)0x74,(char)0x7C,(char)0x34,(char)0x03,(char)0x6F,(char)0x72,(char)0x74,(char)0xFF,(char)0x33,(char)0x00
      ,(char)0x19,(char)0x69,(char)0x0D,(char)0x79,(char)0x73,(char)0x74,(char)0x65,(char)0x6D,(char)0x55,(char)0x70
      ,(char)0x64,(char)0x61,(char)0x74,(char)0x65,(char)0x49,(char)0x44,(char)0xE5,(char)0x9C,(char)0x02,(char)0x49
      ,(char)0x64,(char)0x38,(char)0x66,(char)0x00,(char)0x0F,(char)0x1B,(char)0x00,(char)0x3F,(char)0xCD,(char)0x00
      ,(char)0xCD,(char)0x8B,(char)0x00,(char)0x65,(char)0xCB,(char)0x09,(char)0x43,(char)0x6F,(char)0x6E,(char)0x74
      ,(char)0x61,(char)0x69,(char)0x6E,(char)0x65,(char)0x72,(char)0x3F,(char)0xCC,(char)0x00,(char)0x2F,(char)0xCC
      ,(char)0x00,(char)0x16,(char)0xB8,(char)0x07,(char)0x72,(char)0x69,(char)0x74,(char)0x65,(char)0x72,(char)0x69
      ,(char)0x61,(char)0x7F,(char)0xEE,(char)0x02,(char)0x45,(char)0x5F,(char)0xD0,(char)0x13,(char)0x00,(char)0xF0
      ,(char)0x23,(char)0x06,(char)0x46,(char)0x69,(char)0x6C,(char)0x74,(char)0x65,(char)0x72,(char)0x3F,(char)0x44
      ,(char)0x02,(char)0x45,(char)0x5F,(char)0xC8,(char)0x11,(char)0x00,(char)0xB1,(char)0x43,(char)0x0C,(char)0x74
      ,(char)0x61,(char)0x72,(char)0x74,(char)0x69,(char)0x6E,(char)0x67,(char)0x49,(char)0x6E,(char)0x64,(char)0x65
      ,(char)0x78,(char)0xBF,(char)0x65,(char)0x02,(char)0x45,(char)0x5F,(char)0x87,(char)0x11,(char)0x00,(char)0xF0
      ,(char)0x64,(char)0x0E,(char)0x52,(char)0x65,(char)0x71,(char)0x75,(char)0x65,(char)0x73,(char)0x74,(char)0x65
      ,(char)0x64,(char)0x43,(char)0x6F,(char)0x75,(char)0x6E,(char)0x74,(char)0x3F,(char)0x87,(char)0x02,(char)0x45
      ,(char)0x5F,(char)0x87,(char)0x11,(char)0x00,(char)0x71,(char)0x86,(char)0x03,(char)0x6F,(char)0x72,(char)0x74
      ,(char)0xFF,(char)0x85,(char)0x00,(char)0xCB,(char)0x85,(char)0x03,(char)0x6F,(char)0x72,(char)0x74,(char)0x7A
      ,(char)0x85,(char)0x06,(char)0x52,(char)0x65,(char)0x73,(char)0x75,(char)0x6C,(char)0x74,(char)0x37,(char)0xFA
      ,(char)0x00,(char)0x8B,(char)0xC9,(char)0x00,(char)0x08,(char)0x12,(char)0x00,(char)0x30,(char)0xC9,(char)0x0E
      ,(char)0x4E,(char)0x75,(char)0x6D,(char)0x62,(char)0x65,(char)0x72,(char)0x52,(char)0x65,(char)0x74,(char)0x75
      ,(char)0x72,(char)0x6E,(char)0x65,(char)0x64,(char)0x3F,(char)0x22,(char)0x00,(char)0xBA,(char)0x64,(char)0x0C
      ,(char)0x54,(char)0x6F,(char)0x74,(char)0x61,(char)0x6C,(char)0x4D,(char)0x61,(char)0x74,(char)0x63,(char)0x68
      ,(char)0x65,(char)0x73,(char)0x7F,(char)0x21,(char)0x00,(char)0xFA,(char)0x85,(char)0x08,(char)0x55,(char)0x70
      ,(char)0x64,(char)0x61,(char)0x74,(char)0x65,(char)0x49,(char)0x44,(char)0xBF,(char)0x63,(char)0x03,(char)0x50
      ,(char)0x45,(char)0x5F,(char)0x8A,(char)0x12,(char)0x00,(char)0xA1,(char)0xE9,(char)0x00,(char)0x49,(char)0xEC
      ,(char)0x03,(char)0x4C,(char)0x69,(char)0x73,(char)0xC5,(char)0x03,(char)0x00,(char)0xC6,(char)0xE3
   },
   709,
   4209
};
struct MediaServer__Action_ContentDirectory_Browse MediaServer__Action_ContentDirectory_Browse_Impl = 
{
   0,
   1392
};
struct MediaServer__Action_ContentDirectory_CreateObject MediaServer__Action_ContentDirectory_CreateObject_Impl = 
{
   1392,
   594
};
struct MediaServer__Action_ContentDirectory_DestroyObject MediaServer__Action_ContentDirectory_DestroyObject_Impl = 
{
   1986,
   203
};
struct MediaServer__Action_ContentDirectory_GetSearchCapabilities MediaServer__Action_ContentDirectory_GetSearchCapabilities_Impl = 
{
   2189,
   213
};
struct MediaServer__Action_ContentDirectory_GetSortCapabilities MediaServer__Action_ContentDirectory_GetSortCapabilities_Impl = 
{
   2402,
   207
};
struct MediaServer__Action_ContentDirectory_GetSystemUpdateID MediaServer__Action_ContentDirectory_GetSystemUpdateID_Impl = 
{
   2609,
   197
};
struct MediaServer__Action_ContentDirectory_Search MediaServer__Action_ContentDirectory_Search_Impl = 
{
   2806,
   1403
};
struct MediaServer__Service_ConnectionManager MediaServer__Service_ConnectionManager_Impl =
{
   &MediaServer__Action_ConnectionManager_GetCurrentConnectionIDs_Impl,
   &MediaServer__Action_ConnectionManager_GetCurrentConnectionInfo_Impl,
   &MediaServer__Action_ConnectionManager_GetProtocolInfo_Impl,
   &MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_ProtocolInfo_Impl,
   &MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_ConnectionStatus_Impl,
   &MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_AVTransportID_Impl,
   &MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_RcsID_Impl,
   &MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_ConnectionID_Impl,
   &MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_ConnectionManager_Impl,
   &MediaServer__StateVariable_ConnectionManager_SourceProtocolInfo_Impl,
   &MediaServer__StateVariable_ConnectionManager_SinkProtocolInfo_Impl,
   &MediaServer__StateVariable_ConnectionManager_A_ARG_TYPE_Direction_Impl,
   &MediaServer__StateVariable_ConnectionManager_CurrentConnectionIDs_Impl,
   {
      (char)0x09,(char)0x3C,(char)0x73,(char)0x65,(char)0x72,(char)0x76,(char)0x69,(char)0x63,(char)0x65,(char)0x3E
      ,(char)0x48,(char)0x02,(char)0x1A,(char)0x54,(char)0x79,(char)0x70,(char)0x65,(char)0x3E,(char)0x75,(char)0x72
      ,(char)0x6E,(char)0x3A,(char)0x73,(char)0x63,(char)0x68,(char)0x65,(char)0x6D,(char)0x61,(char)0x73,(char)0x2D
      ,(char)0x75,(char)0x70,(char)0x6E,(char)0x70,(char)0x2D,(char)0x6F,(char)0x72,(char)0x67,(char)0x3A,(char)0x87
      ,(char)0x0A,(char)0x16,(char)0x3A,(char)0x43,(char)0x6F,(char)0x6E,(char)0x6E,(char)0x65,(char)0x63,(char)0x74
      ,(char)0x69,(char)0x6F,(char)0x6E,(char)0x4D,(char)0x61,(char)0x6E,(char)0x61,(char)0x67,(char)0x65,(char)0x72
      ,(char)0x3A,(char)0x31,(char)0x3C,(char)0x2F,(char)0x8C,(char)0x0F,(char)0x00,(char)0x08,(char)0x15,(char)0x02
      ,(char)0x49,(char)0x64,(char)0x45,(char)0x12,(char)0x00,(char)0x50,(char)0x10,(char)0x02,(char)0x49,(char)0x64
      ,(char)0xD2,(char)0x10,(char)0x02,(char)0x3C,(char)0x2F,(char)0x0A,(char)0x0D,(char)0x09,(char)0x3C,(char)0x53
      ,(char)0x43,(char)0x50,(char)0x44,(char)0x55,(char)0x52,(char)0x4C,(char)0x3E,(char)0x51,(char)0x1A,(char)0x0B
      ,(char)0x2F,(char)0x73,(char)0x63,(char)0x70,(char)0x64,(char)0x2E,(char)0x78,(char)0x6D,(char)0x6C,(char)0x3C
      ,(char)0x2F,(char)0x08,(char)0x09,(char)0x08,(char)0x3C,(char)0x63,(char)0x6F,(char)0x6E,(char)0x74,(char)0x72
      ,(char)0x6F,(char)0x6C,(char)0x16,(char)0x0C,(char)0x00,(char)0x47,(char)0x07,(char)0x02,(char)0x3C,(char)0x2F
      ,(char)0x8B,(char)0x09,(char)0x09,(char)0x3C,(char)0x65,(char)0x76,(char)0x65,(char)0x6E,(char)0x74,(char)0x53
      ,(char)0x75,(char)0x62,(char)0xD6,(char)0x18,(char)0x00,(char)0x85,(char)0x07,(char)0x02,(char)0x3C,(char)0x2F
      ,(char)0x4C,(char)0x09,(char)0x00,(char)0x89,(char)0x37,(char)0x01,(char)0x3E,(char)0x00,(char)0x00
   },
   159,
   302,
};
struct MediaServer__Service_ContentDirectory MediaServer__Service_ContentDirectory_Impl =
{
   &MediaServer__Action_ContentDirectory_Browse_Impl,
   &MediaServer__Action_ContentDirectory_CreateObject_Impl,
   &MediaServer__Action_ContentDirectory_DestroyObject_Impl,
   &MediaServer__Action_ContentDirectory_GetSearchCapabilities_Impl,
   &MediaServer__Action_ContentDirectory_GetSortCapabilities_Impl,
   &MediaServer__Action_ContentDirectory_GetSystemUpdateID_Impl,
   &MediaServer__Action_ContentDirectory_Search_Impl,
   &MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_BrowseFlag_Impl,
   &MediaServer__StateVariable_ContentDirectory_ContainerUpdateIDs_Impl,
   &MediaServer__StateVariable_ContentDirectory_SystemUpdateID_Impl,
   &MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_SortCriteria_Impl,
   &MediaServer__StateVariable_ContentDirectory_SortCapabilities_Impl,
   &MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_UpdateID_Impl,
   &MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_Index_Impl,
   &MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_ObjectID_Impl,
   &MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_Count_Impl,
   &MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_Result_Impl,
   &MediaServer__StateVariable_ContentDirectory_SearchCapabilities_Impl,
   &MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_SearchCriteria_Impl,
   &MediaServer__StateVariable_ContentDirectory_A_ARG_TYPE_Filter_Impl,
   {
      (char)0x09,(char)0x3C,(char)0x73,(char)0x65,(char)0x72,(char)0x76,(char)0x69,(char)0x63,(char)0x65,(char)0x3E
      ,(char)0x48,(char)0x02,(char)0x1A,(char)0x54,(char)0x79,(char)0x70,(char)0x65,(char)0x3E,(char)0x75,(char)0x72
      ,(char)0x6E,(char)0x3A,(char)0x73,(char)0x63,(char)0x68,(char)0x65,(char)0x6D,(char)0x61,(char)0x73,(char)0x2D
      ,(char)0x75,(char)0x70,(char)0x6E,(char)0x70,(char)0x2D,(char)0x6F,(char)0x72,(char)0x67,(char)0x3A,(char)0x87
      ,(char)0x0A,(char)0x15,(char)0x3A,(char)0x43,(char)0x6F,(char)0x6E,(char)0x74,(char)0x65,(char)0x6E,(char)0x74
      ,(char)0x44,(char)0x69,(char)0x72,(char)0x65,(char)0x63,(char)0x74,(char)0x6F,(char)0x72,(char)0x79,(char)0x3A
      ,(char)0x31,(char)0x3C,(char)0x2F,(char)0x4C,(char)0x0F,(char)0x00,(char)0xC8,(char)0x14,(char)0x02,(char)0x49
      ,(char)0x64,(char)0x05,(char)0x12,(char)0x00,(char)0x10,(char)0x10,(char)0x02,(char)0x49,(char)0x64,(char)0x91
      ,(char)0x10,(char)0x02,(char)0x3C,(char)0x2F,(char)0xCA,(char)0x0C,(char)0x09,(char)0x3C,(char)0x53,(char)0x43
      ,(char)0x50,(char)0x44,(char)0x55,(char)0x52,(char)0x4C,(char)0x3E,(char)0xD0,(char)0x19,(char)0x0B,(char)0x2F
      ,(char)0x73,(char)0x63,(char)0x70,(char)0x64,(char)0x2E,(char)0x78,(char)0x6D,(char)0x6C,(char)0x3C,(char)0x2F
      ,(char)0xC8,(char)0x08,(char)0x08,(char)0x3C,(char)0x63,(char)0x6F,(char)0x6E,(char)0x74,(char)0x72,(char)0x6F
      ,(char)0x6C,(char)0xD5,(char)0x0B,(char)0x00,(char)0x07,(char)0x07,(char)0x02,(char)0x3C,(char)0x2F,(char)0x4B
      ,(char)0x09,(char)0x09,(char)0x3C,(char)0x65,(char)0x76,(char)0x65,(char)0x6E,(char)0x74,(char)0x53,(char)0x75
      ,(char)0x62,(char)0x55,(char)0x18,(char)0x00,(char)0x45,(char)0x07,(char)0x02,(char)0x3C,(char)0x2F,(char)0x0C
      ,(char)0x09,(char)0x00,(char)0x89,(char)0x36,(char)0x01,(char)0x3E,(char)0x00,(char)0x00
   },
   158,
   297,
};
struct MediaServer__Device_MediaServer MediaServer__Device_MediaServer_Impl = 
{
   &MediaServer__Service_ConnectionManager_Impl,
   &MediaServer__Service_ContentDirectory_Impl,
   
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   {
      (char)0x61,(char)0x3C,(char)0x3F,(char)0x78,(char)0x6D,(char)0x6C,(char)0x20,(char)0x76,(char)0x65,(char)0x72
      ,(char)0x73,(char)0x69,(char)0x6F,(char)0x6E,(char)0x3D,(char)0x22,(char)0x31,(char)0x2E,(char)0x30,(char)0x22
      ,(char)0x20,(char)0x65,(char)0x6E,(char)0x63,(char)0x6F,(char)0x64,(char)0x69,(char)0x6E,(char)0x67,(char)0x3D
      ,(char)0x22,(char)0x75,(char)0x74,(char)0x66,(char)0x2D,(char)0x38,(char)0x22,(char)0x3F,(char)0x3E,(char)0x0D
      ,(char)0x0A,(char)0x3C,(char)0x72,(char)0x6F,(char)0x6F,(char)0x74,(char)0x20,(char)0x78,(char)0x6D,(char)0x6C
      ,(char)0x6E,(char)0x73,(char)0x3D,(char)0x22,(char)0x75,(char)0x72,(char)0x6E,(char)0x3A,(char)0x73,(char)0x63
      ,(char)0x68,(char)0x65,(char)0x6D,(char)0x61,(char)0x73,(char)0x2D,(char)0x75,(char)0x70,(char)0x6E,(char)0x70
      ,(char)0x2D,(char)0x6F,(char)0x72,(char)0x67,(char)0x3A,(char)0x64,(char)0x65,(char)0x76,(char)0x69,(char)0x63
      ,(char)0x65,(char)0x2D,(char)0x31,(char)0x2D,(char)0x30,(char)0x22,(char)0x3E,(char)0x0D,(char)0x0A,(char)0x20
      ,(char)0x20,(char)0x20,(char)0x3C,(char)0x73,(char)0x70,(char)0x65,(char)0x63,(char)0x56,(char)0x86,(char)0x16
      ,(char)0x00,(char)0x86,(char)0x04,(char)0x00,(char)0x44,(char)0x05,(char)0x09,(char)0x6D,(char)0x61,(char)0x6A
      ,(char)0x6F,(char)0x72,(char)0x3E,(char)0x31,(char)0x3C,(char)0x2F,(char)0x46,(char)0x02,(char)0x00,(char)0x0A
      ,(char)0x06,(char)0x0B,(char)0x69,(char)0x6E,(char)0x6F,(char)0x72,(char)0x3E,(char)0x30,(char)0x3C,(char)0x2F
      ,(char)0x6D,(char)0x69,(char)0x6E,(char)0x08,(char)0x06,(char)0x02,(char)0x3C,(char)0x2F,(char)0xD1,(char)0x10
      ,(char)0x01,(char)0x3C,(char)0xC6,(char)0x19,(char)0x00,(char)0x0A,(char)0x14,(char)0x00,(char)0xC6,(char)0x1D
      ,(char)0x05,(char)0x54,(char)0x79,(char)0x70,(char)0x65,(char)0x3E,(char)0xDB,(char)0x25,(char)0x10,(char)0x3A
      ,(char)0x4D,(char)0x65,(char)0x64,(char)0x69,(char)0x61,(char)0x53,(char)0x65,(char)0x72,(char)0x76,(char)0x65
      ,(char)0x72,(char)0x3A,(char)0x31,(char)0x3C,(char)0x2F,(char)0x8B,(char)0x0D,(char)0x00,(char)0x8A,(char)0x12
      ,(char)0x0D,(char)0x6C,(char)0x6E,(char)0x61,(char)0x3A,(char)0x58,(char)0x5F,(char)0x44,(char)0x4C,(char)0x4E
      ,(char)0x41,(char)0x43,(char)0x41,(char)0x50,(char)0x06,(char)0x3B,(char)0x01,(char)0x3A,(char)0x44,(char)0x05
      ,(char)0x00,(char)0x4E,(char)0x3C,(char)0x00,(char)0xC4,(char)0x09,(char)0x00,(char)0x51,(char)0x3C,(char)0x12
      ,(char)0x69,(char)0x6D,(char)0x61,(char)0x67,(char)0x65,(char)0x2D,(char)0x75,(char)0x70,(char)0x6C,(char)0x6F
      ,(char)0x61,(char)0x64,(char)0x2C,(char)0x61,(char)0x75,(char)0x64,(char)0x69,(char)0x6F,(char)0x49,(char)0x03
      ,(char)0x01,(char)0x76,(char)0xC8,(char)0x05,(char)0x18,(char)0x63,(char)0x72,(char)0x65,(char)0x61,(char)0x74
      ,(char)0x65,(char)0x2D,(char)0x63,(char)0x68,(char)0x69,(char)0x6C,(char)0x64,(char)0x2D,(char)0x63,(char)0x6F
      ,(char)0x6E,(char)0x74,(char)0x61,(char)0x69,(char)0x6E,(char)0x65,(char)0x72,(char)0x3C,(char)0x2F,(char)0x0E
      ,(char)0x1E,(char)0x00,(char)0x15,(char)0x24,(char)0x03,(char)0x44,(char)0x4F,(char)0x43,(char)0x2E,(char)0x24
      ,(char)0x0A,(char)0x44,(char)0x4D,(char)0x53,(char)0x2D,(char)0x31,(char)0x2E,(char)0x35,(char)0x30,(char)0x3C
      ,(char)0x2F,(char)0x8E,(char)0x11,(char)0x00,(char)0x0A,(char)0x62,(char)0x11,(char)0x66,(char)0x72,(char)0x69
      ,(char)0x65,(char)0x6E,(char)0x64,(char)0x6C,(char)0x79,(char)0x4E,(char)0x61,(char)0x6D,(char)0x65,(char)0x3E
      ,(char)0x25,(char)0x73,(char)0x3C,(char)0x2F,(char)0x4D,(char)0x04,(char)0x00,(char)0xCB,(char)0x6B,(char)0x0A
      ,(char)0x6E,(char)0x75,(char)0x66,(char)0x61,(char)0x63,(char)0x74,(char)0x75,(char)0x72,(char)0x65,(char)0x72
      ,(char)0xC5,(char)0x09,(char)0x00,(char)0x4D,(char)0x04,(char)0x00,(char)0xD5,(char)0x09,(char)0x03,(char)0x55
      ,(char)0x52,(char)0x4C,(char)0x91,(char)0x0A,(char)0x03,(char)0x55,(char)0x52,(char)0x4C,(char)0xCB,(char)0x80
      ,(char)0x0F,(char)0x6F,(char)0x64,(char)0x65,(char)0x6C,(char)0x44,(char)0x65,(char)0x73,(char)0x63,(char)0x72
      ,(char)0x69,(char)0x70,(char)0x74,(char)0x69,(char)0x6F,(char)0x6E,(char)0x06,(char)0x16,(char)0x00,(char)0x50
      ,(char)0x05,(char)0x00,(char)0xCE,(char)0x0B,(char)0x00,(char)0xC9,(char)0x29,(char)0x00,(char)0x8A,(char)0x03
      ,(char)0x00,(char)0x4F,(char)0x08,(char)0x05,(char)0x75,(char)0x6D,(char)0x62,(char)0x65,(char)0x72,(char)0xCB
      ,(char)0x08,(char)0x03,(char)0x75,(char)0x6D,(char)0x62,(char)0x8D,(char)0x28,(char)0x00,(char)0x44,(char)0x1D
      ,(char)0x00,(char)0xC4,(char)0x26,(char)0x0B,(char)0x68,(char)0x74,(char)0x74,(char)0x70,(char)0x3A,(char)0x2F
      ,(char)0x2F,(char)0x32,(char)0x35,(char)0x35,(char)0x2E,(char)0x04,(char)0x01,(char)0x00,(char)0x07,(char)0x02
      ,(char)0x07,(char)0x3A,(char)0x32,(char)0x35,(char)0x35,(char)0x2F,(char)0x3C,(char)0x2F,(char)0x89,(char)0x09
      ,(char)0x00,(char)0x09,(char)0xAC,(char)0x05,(char)0x73,(char)0x65,(char)0x72,(char)0x69,(char)0x61,(char)0x8C
      ,(char)0x17,(char)0x00,(char)0x4D,(char)0x04,(char)0x00,(char)0xC9,(char)0xB5,(char)0x09,(char)0x55,(char)0x44
      ,(char)0x4E,(char)0x3E,(char)0x75,(char)0x75,(char)0x69,(char)0x64,(char)0x3A,(char)0xC4,(char)0x52,(char)0x03
      ,(char)0x55,(char)0x44,(char)0x4E,(char)0x48,(char)0xB0,(char)0x00,(char)0xC9,(char)0xAB,(char)0x02,(char)0x3C
      ,(char)0x2F,(char)0x84,(char)0xD0,(char)0x01,(char)0x3E,(char)0x00,(char)0x00
   },
   467,
   880
};

MediaServer_MicroStackToken MediaServer_CreateMicroStack(void *Chain, const char* FriendlyName,const char* UDN, const char* SerialNumber, const int NotifyCycleSeconds, const unsigned short PortNum);
/* UPnP Set Function Pointers Methods */
void (*MediaServer_FP_PresentationPage) (void* upnptoken,struct packetheader *packet);
/*! \var MediaServer_FP_ConnectionManager_GetCurrentConnectionIDs
	\brief Dispatch Pointer for ConnectionManager >> urn:schemas-upnp-org:service:ConnectionManager:1 >> GetCurrentConnectionIDs
*/
MediaServer__ActionHandler_ConnectionManager_GetCurrentConnectionIDs MediaServer_FP_ConnectionManager_GetCurrentConnectionIDs;
/*! \var MediaServer_FP_ConnectionManager_GetCurrentConnectionInfo
	\brief Dispatch Pointer for ConnectionManager >> urn:schemas-upnp-org:service:ConnectionManager:1 >> GetCurrentConnectionInfo
*/
MediaServer__ActionHandler_ConnectionManager_GetCurrentConnectionInfo MediaServer_FP_ConnectionManager_GetCurrentConnectionInfo;
/*! \var MediaServer_FP_ConnectionManager_GetProtocolInfo
	\brief Dispatch Pointer for ConnectionManager >> urn:schemas-upnp-org:service:ConnectionManager:1 >> GetProtocolInfo
*/
MediaServer__ActionHandler_ConnectionManager_GetProtocolInfo MediaServer_FP_ConnectionManager_GetProtocolInfo;
/*! \var MediaServer_FP_ContentDirectory_Browse
	\brief Dispatch Pointer for ContentDirectory >> urn:schemas-upnp-org:service:ContentDirectory:1 >> Browse
*/
MediaServer__ActionHandler_ContentDirectory_Browse MediaServer_FP_ContentDirectory_Browse;
/*! \var MediaServer_FP_ContentDirectory_CreateObject
	\brief Dispatch Pointer for ContentDirectory >> urn:schemas-upnp-org:service:ContentDirectory:1 >> CreateObject
*/
MediaServer__ActionHandler_ContentDirectory_CreateObject MediaServer_FP_ContentDirectory_CreateObject;
/*! \var MediaServer_FP_ContentDirectory_DestroyObject
	\brief Dispatch Pointer for ContentDirectory >> urn:schemas-upnp-org:service:ContentDirectory:1 >> DestroyObject
*/
MediaServer__ActionHandler_ContentDirectory_DestroyObject MediaServer_FP_ContentDirectory_DestroyObject;
/*! \var MediaServer_FP_ContentDirectory_GetSearchCapabilities
	\brief Dispatch Pointer for ContentDirectory >> urn:schemas-upnp-org:service:ContentDirectory:1 >> GetSearchCapabilities
*/
MediaServer__ActionHandler_ContentDirectory_GetSearchCapabilities MediaServer_FP_ContentDirectory_GetSearchCapabilities;
/*! \var MediaServer_FP_ContentDirectory_GetSortCapabilities
	\brief Dispatch Pointer for ContentDirectory >> urn:schemas-upnp-org:service:ContentDirectory:1 >> GetSortCapabilities
*/
MediaServer__ActionHandler_ContentDirectory_GetSortCapabilities MediaServer_FP_ContentDirectory_GetSortCapabilities;
/*! \var MediaServer_FP_ContentDirectory_GetSystemUpdateID
	\brief Dispatch Pointer for ContentDirectory >> urn:schemas-upnp-org:service:ContentDirectory:1 >> GetSystemUpdateID
*/
MediaServer__ActionHandler_ContentDirectory_GetSystemUpdateID MediaServer_FP_ContentDirectory_GetSystemUpdateID;
/*! \var MediaServer_FP_ContentDirectory_Search
	\brief Dispatch Pointer for ContentDirectory >> urn:schemas-upnp-org:service:ContentDirectory:1 >> Search
*/
MediaServer__ActionHandler_ContentDirectory_Search MediaServer_FP_ContentDirectory_Search;





struct MediaServer_DataObject;

//
// It should not be necessary to expose/modify any of these structures. They
// are used by the internal stack
//

struct SubscriberInfo
{
   char* SID;		// Subscription ID
   int SIDLength;
   int SEQ;
   
   
   int Address;
   unsigned short Port;
   char* Path;
   int PathLength;
   int RefCount;
   int Disposing;
   
   struct timeval RenewByTime;
   
   struct SubscriberInfo *Next;
   struct SubscriberInfo *Previous;
};
struct MediaServer_DataObject
{
   //
   // Absolutely DO NOT put anything above these 3 function pointers
   //
   ILibChain_PreSelect PreSelect;
   ILibChain_PostSelect PostSelect;
   ILibChain_Destroy Destroy;
   
   void *EventClient;
   void *Chain;
   int UpdateFlag;
   
   /* Network Poll */
   unsigned int NetworkPollTime;
   
   int ForceExit;
   char *UUID;
   char *UDN;
   char *Serial;
   void *User;
   
   void *WebServerTimer;
   void *HTTPServer;
   
   int InitialNotify;
   
   char* ConnectionManager_SourceProtocolInfo;
	char* ConnectionManager_SinkProtocolInfo;
	char* ConnectionManager_CurrentConnectionIDs;
	char* ContentDirectory_ContainerUpdateIDs;
	char* ContentDirectory_SystemUpdateID;

   
   struct sockaddr_in addr;
   int addrlen;
   
   struct ip_mreq mreq;
   int *AddressList;
   int AddressListLength;
   
   int _NumEmbeddedDevices;
   int WebSocketPortNumber;
   
   void **NOTIFY_RECEIVE_socks;
   void **NOTIFY_SEND_socks;
   
   
   
   struct timeval CurrentTime;
   struct timeval NotifyTime;
   
   int SID;
   int NotifyCycleTime;
   
   
   
   sem_t EventLock;
   struct SubscriberInfo *HeadSubscriberPtr_ConnectionManager;
	int NumberOfSubscribers_ConnectionManager;
	struct SubscriberInfo *HeadSubscriberPtr_ContentDirectory;
	int NumberOfSubscribers_ContentDirectory;

};

struct MSEARCH_state
{
   char *ST;
   int STLength;
   void *upnp;
   struct sockaddr_in dest_addr;
   int localIPAddress;
};
struct MediaServer_FragmentNotifyStruct
{
   struct MediaServer_DataObject *upnp;
   int packetNumber;
};

/* Pre-declarations */

void MediaServer_StreamDescriptionDocument(struct ILibWebServer_Session *session);

void MediaServer_FragmentedSendNotify(void *data);
void MediaServer_SendNotify(const struct MediaServer_DataObject *upnp);
void MediaServer_SendByeBye(const struct MediaServer_DataObject *upnp);
void MediaServer_MainInvokeSwitch();
void MediaServer_SendDataXmlEscaped(const void* MediaServer_Token, const char* Data, const int DataLength, const int Terminate);
void MediaServer_SendData(const void* MediaServer_Token, const char* Data, const int DataLength, const int Terminate);
int MediaServer_PeriodicNotify(struct MediaServer_DataObject *upnp);
void MediaServer_SendEvent_Body(void *upnptoken, char *body, int bodylength, struct SubscriberInfo *info);
void MediaServer_ProcessMSEARCH(struct MediaServer_DataObject *upnp, struct packetheader *packet);
struct in_addr MediaServer__inaddr;

/*! \fn MediaServer_GetWebServerToken(const MediaServer_MicroStackToken MicroStackToken)
\brief Converts a MicroStackToken to a WebServerToken
\par
\a MicroStackToken is the void* returned from a call to MediaServer_CreateMicroStack. The returned token, is the server token
not the session token.
\param MicroStackToken MicroStack Token
\returns WebServer Token
*/
void* MediaServer_GetWebServerToken(const MediaServer_MicroStackToken MicroStackToken)
{
   return(((struct MediaServer_DataObject*)MicroStackToken)->HTTPServer);
}





#define MediaServer_BuildSsdpResponsePacket(outpacket,outlength,ipaddr,port,EmbeddedDeviceNumber,USN,USNex,ST,NTex,NotifyTime)\
{\
   MediaServer__inaddr.s_addr = ipaddr;\
   *outlength = sprintf(outpacket,"HTTP/1.1 200 OK\r\nLOCATION: http://%s:%d/\r\nEXT:\r\nSERVER: %s, UPnP/1.0, Intel MicroStack/1.0.2384\r\nUSN: uuid:%s%s\r\nCACHE-CONTROL: max-age=%d\r\nST: %s%s\r\n\r\n" ,inet_ntoa(MediaServer__inaddr),port,MediaServer_PLATFORM,USN,USNex,NotifyTime,ST,NTex);\
}
#define MediaServer_BuildSsdpNotifyPacket(outpacket,outlength,ipaddr,port,EmbeddedDeviceNumber,USN,USNex,NT,NTex,NotifyTime)\
{\
   MediaServer__inaddr.s_addr = ipaddr;\
   *outlength = sprintf(outpacket,"NOTIFY * HTTP/1.1\r\nLOCATION: http://%s:%d/\r\nHOST: 239.255.255.250:1900\r\nSERVER: %s, UPnP/1.0, Intel MicroStack/1.0.2384\r\nNTS: ssdp:alive\r\nUSN: uuid:%s%s\r\nCACHE-CONTROL: max-age=%d\r\nNT: %s%s\r\n\r\n",inet_ntoa(MediaServer__inaddr),port,MediaServer_PLATFORM,USN,USNex,NotifyTime,NT,NTex);\
}




void MediaServer_SetDisconnectFlag(MediaServer_SessionToken token,void *flag)
{
   ((struct ILibWebServer_Session*)token)->Reserved10=flag;
}

/*! \defgroup FragmentResponse Fragmented Response System
\ingroup MicroStack
\brief Methods used by application to response to invocations in a fragmented manner.
\par
Typically an application will use one of the \a MediaServer_Response_ methods to resond to an invocation
request. However, that requires that all of the arguments are known at the time that method is called.
There are times when the application may not be able to do that, such as when querying a back-end
server.<br><br>
In this case, the application can utilise the Fragmented Response System. The application would need
to call \a MediaServer_AsyncResponse_START exactly once to initialise the response. Then the application would
repeatedly call \a MediaServer_AsyncResponse_OUT for each argument in the response. Then finally, a call to
\a MediaServer_AsyncResponse_DONE to complete
\{
   */
   /*! \fn MediaServer_AsyncResponse_START(const MediaServer_SessionToken MediaServer_Token, const char* actionName, const char* serviceUrnWithVersion)
   \brief Fragmented Response Initializer
   \param MediaServer_Token The UPnP token received in the invocation request
   \param actionName The name of the method this response is for
   \param serviceUrnWithVersion The full service type URN, this response is for.
   \returns Send Status
   */
   int MediaServer_AsyncResponse_START(const MediaServer_SessionToken MediaServer_Token, const char* actionName, const char* serviceUrnWithVersion)
   {
      int RetVal=0;
      #if defined(WIN32) || defined(_WIN32_WCE)
      char* RESPONSE_HEADER = "\r\nEXT:\r\nCONTENT-TYPE: text/xml; charset=\"utf-8\"\r\nSERVER: WINDOWS, UPnP/1.0, Intel MicroStack/1.0.2384";
      #else
      char* RESPONSE_HEADER = "\r\nEXT:\r\nCONTENT-TYPE: text/xml; charset=\"utf-8\"\r\nSERVER: POSIX, UPnP/1.0, Intel MicroStack/1.0.2384";
      #endif
      char* RESPONSE_BODY = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n<s:Body>\r\n<u:%sResponse xmlns:u=\"%s\">";
      struct ILibWebServer_Session *session = (struct ILibWebServer_Session*)MediaServer_Token;
      
      int headSize;
      char* head; 
      int headLength;
      
      if(session==NULL){return(1);}
      
      headSize = (int)strlen(RESPONSE_BODY) + (int)strlen(actionName) + (int)strlen(serviceUrnWithVersion) + 1;
      head = (char*) malloc (headSize);
      headLength = sprintf(head, RESPONSE_BODY, actionName, serviceUrnWithVersion);
      
      RetVal = ILibWebServer_StreamHeader_Raw(session,200,"OK",RESPONSE_HEADER,1);
      if(RetVal>=0)
      {
         RetVal = ILibWebServer_StreamBody(session,head,headLength,0,0);
      }
      else
      {
         free(head);
      }
      return(RetVal);
   }
   /*! \fn MediaServer_AsyncResponse_DONE(const MediaServer_SessionToken MediaServer_Token, const char* actionName)
   \brief Fragmented Response Finalizer
   \param MediaServer_Token The UPnP token received in the invocation request
   \param actionName The name of the method this response is for
   \returns Send Status
   */
   int MediaServer_AsyncResponse_DONE(const MediaServer_SessionToken MediaServer_Token, const char* actionName)
   {
      char* RESPONSE_FOOTER = "</u:%sResponse>\r\n   </s:Body>\r\n</s:Envelope>";
      
      int footSize = (int)strlen(RESPONSE_FOOTER) + (int)strlen(actionName);
      struct ILibWebServer_Session *session = (struct ILibWebServer_Session*)MediaServer_Token;
      char* footer;
      int footLength;
      
      footer = (char*) malloc(footSize);
      footLength = sprintf(footer, RESPONSE_FOOTER, actionName);
      
      return(ILibWebServer_StreamBody(session,footer,footLength,0,1));
   }
   /*! \fn MediaServer_AsyncResponse_OUT(const MediaServer_SessionToken MediaServer_Token, const char* outArgName, const char* bytes, const int byteLength, enum ILibAsyncSocket_MemoryOwnership bytesMemoryOwnership,const int startArg, const int endArg)
   \brief Fragmented Response Data
   \param MediaServer_Token The UPnP token received in the invocation request
   \param outArgName Variable Name
   \param bytes Variable Data \b Note: For string types, this MUST be escaped.
   \param byteLength Length of \a bytes
   \param bytesMemoryOwnership Memory Ownership flag for \a bytes
   \param startArg Boolean. 1 to start response, 0 to continue response
   \param endArg Boolean. 1 to finish response, 0 to continue response
   \returns Send Status
   */
   int MediaServer_AsyncResponse_OUT(const MediaServer_SessionToken MediaServer_Token, const char* outArgName, const char* bytes, const int byteLength, enum ILibAsyncSocket_MemoryOwnership bytesMemoryOwnership,const int startArg, const int endArg)
   {
      struct ILibWebServer_Session *session = (struct ILibWebServer_Session*)MediaServer_Token;
      int RetVal=0;
      
      if (startArg != 0)
      {
         RetVal = ILibWebServer_StreamBody(session,"<",1,1,0);
         if(RetVal>=0)
         {
            RetVal = ILibWebServer_StreamBody(session,(char*)outArgName,(int)strlen(outArgName),1,0);
         }
         if(RetVal>=0)
         {
            RetVal = ILibWebServer_StreamBody(session,">",1,1,0);
         }
      }
      
      if(byteLength>0 && RetVal >= 0)
      {
         RetVal = ILibWebServer_StreamBody(session,(char*)bytes,byteLength,bytesMemoryOwnership,0);
      }
      
      if (endArg != 0 && RetVal >= 0)
      {
         RetVal = ILibWebServer_StreamBody(session,"</",2,1,0);
         if(RetVal>=0)
         {
            RetVal = ILibWebServer_StreamBody(session,(char*)outArgName,(int)strlen(outArgName),1,0);
         }
         if(RetVal>=0)
         {
            RetVal = ILibWebServer_StreamBody(session,">\r\n",3,1,0);
         }
      }
      return(RetVal);
   }
   /*! \} */


/*! \fn MediaServer_IPAddressListChanged(MediaServer_MicroStackToken MicroStackToken)
\brief Tell the underlying MicroStack that an IPAddress may have changed
\param MicroStackToken Microstack
*/
void MediaServer_IPAddressListChanged(MediaServer_MicroStackToken MicroStackToken)
{
   ((struct MediaServer_DataObject*)MicroStackToken)->UpdateFlag = 1;
   ILibForceUnBlockChain(((struct MediaServer_DataObject*)MicroStackToken)->Chain);
}


void MediaServer_SSDPSink(ILibAsyncUDPSocket_SocketModule socketModule,char* buffer, int bufferLength, int remoteInterface, unsigned short remotePort, void *user, void *user2, int *PAUSE)
{
   struct packetheader *packet;
   struct sockaddr_in addr;
   memset(&addr,0,sizeof(struct sockaddr_in));
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = remoteInterface;
   addr.sin_port = htons(remotePort);
   
   packet = ILibParsePacketHeader(buffer,0,bufferLength);
   if(packet!=NULL)
   {
      packet->Source = &addr;
      packet->ReceivingAddress = ILibAsyncUDPSocket_GetLocalInterface(socketModule);
      if(packet->StatusCode==-1 && memcmp(packet->Directive,"M-SEARCH",8)==0)
      {
         //
         // Process the search request with our Multicast M-SEARCH Handler
         //
         MediaServer_ProcessMSEARCH(user, packet);
      }
      ILibDestructPacket(packet);
   }
}

//
//	Internal underlying Initialization, that shouldn't be called explicitely
// 
// <param name="state">State object</param>
// <param name="NotifyCycleSeconds">Cycle duration</param>
// <param name="PortNumber">Port Number</param>
void MediaServer_Init(struct MediaServer_DataObject *state, void *chain, const int NotifyCycleSeconds,const unsigned short PortNumber)
{
   int i;
   
   state->Chain = chain;
   
   /* Setup Notification Timer */
   state->NotifyCycleTime = NotifyCycleSeconds;
   
   
   gettimeofday(&(state->CurrentTime),NULL);
   (state->NotifyTime).tv_sec = (state->CurrentTime).tv_sec  + (state->NotifyCycleTime/2);
   
   memset((char *)&(state->addr), 0, sizeof(state->addr));
   state->addr.sin_family = AF_INET;
   state->addr.sin_addr.s_addr = htonl(INADDR_ANY);
   state->addr.sin_port = (unsigned short)htons(UPNP_PORT);
   state->addrlen = sizeof(state->addr);
   
   
   /* Set up socket */
   state->AddressListLength = ILibGetLocalIPAddressList(&(state->AddressList));
   state->NOTIFY_SEND_socks = (void**)malloc(sizeof(void*)*(state->AddressListLength));
   state->NOTIFY_RECEIVE_socks = (void**)malloc(sizeof(void*)*(state->AddressListLength));
   
   //
   // Iterate through all the current IP Addresses
   //
   for(i=0;i<state->AddressListLength;++i)
   {
      state->NOTIFY_SEND_socks[i] = ILibAsyncUDPSocket_Create(
      chain,
      UPNP_MAX_SSDP_HEADER_SIZE,
      state->AddressList[i],
      UPNP_PORT,
      ILibAsyncUDPSocket_Reuse_SHARED,
      NULL,
      NULL,
      state);
      ILibAsyncUDPSocket_JoinMulticastGroup(
      state->NOTIFY_SEND_socks[i],
      state->AddressList[i],
      inet_addr(UPNP_GROUP));
      
      ILibAsyncUDPSocket_SetMulticastTTL(state->NOTIFY_SEND_socks[i],UPNP_SSDP_TTL);
      
      state->NOTIFY_RECEIVE_socks[i] = ILibAsyncUDPSocket_Create(
      state->Chain,
      UPNP_MAX_SSDP_HEADER_SIZE,
      0,
      UPNP_PORT,
      ILibAsyncUDPSocket_Reuse_SHARED,
      &MediaServer_SSDPSink,
      NULL,
      state);
      ILibAsyncUDPSocket_JoinMulticastGroup(
      state->NOTIFY_RECEIVE_socks[i],
      state->AddressList[i],
      inet_addr(UPNP_GROUP));
      
      
   }
   
}
void MediaServer_PostMX_Destroy(void *object)
{
   struct MSEARCH_state *mss = (struct MSEARCH_state*)object;
   free(mss->ST);
   free(mss);
}

void MediaServer_PostMX_MSEARCH(void *object)
{
   struct MSEARCH_state *mss = (struct MSEARCH_state*)object;
   
   char *b = (char*)malloc(sizeof(char)*5000);
   int packetlength;
   void *response_socket;
   void *subChain;
   
   char *ST = mss->ST;
   int STLength = mss->STLength;
   struct MediaServer_DataObject *upnp = (struct MediaServer_DataObject*)mss->upnp;
   
   subChain = ILibCreateChain();
   
   response_socket = ILibAsyncUDPSocket_Create(
   subChain,
   UPNP_MAX_SSDP_HEADER_SIZE,
   mss->localIPAddress,
   0,
   ILibAsyncUDPSocket_Reuse_SHARED,
   NULL,
   NULL,
   subChain);
   
   
   //
   // Search for root device
   //
   if(STLength==15 && memcmp(ST,"upnp:rootdevice",15)==0)
   {
      
      MediaServer_BuildSsdpResponsePacket(b,&packetlength,mss->localIPAddress,(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::upnp:rootdevice","upnp:rootdevice","",upnp->NotifyCycleTime);
      
      
      ILibAsyncUDPSocket_SendTo(response_socket,mss->dest_addr.sin_addr.s_addr, ntohs(mss->dest_addr.sin_port), b, packetlength, ILibAsyncSocket_MemoryOwnership_USER);
   }
   //
   // Search for everything
   //
   else if(STLength==8 && memcmp(ST,"ssdp:all",8)==0)
   {
      MediaServer_BuildSsdpResponsePacket(b,&packetlength,mss->localIPAddress,(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::upnp:rootdevice","upnp:rootdevice","",upnp->NotifyCycleTime);
							ILibAsyncUDPSocket_SendTo(response_socket,mss->dest_addr.sin_addr.s_addr, ntohs(mss->dest_addr.sin_port), b, packetlength, ILibAsyncSocket_MemoryOwnership_USER);
							MediaServer_BuildSsdpResponsePacket(b,&packetlength,mss->localIPAddress,(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"",upnp->UUID,"",upnp->NotifyCycleTime);
							ILibAsyncUDPSocket_SendTo(response_socket,mss->dest_addr.sin_addr.s_addr, ntohs(mss->dest_addr.sin_port), b, packetlength, ILibAsyncSocket_MemoryOwnership_USER);
						MediaServer_BuildSsdpResponsePacket(b,&packetlength,mss->localIPAddress,(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:device:MediaServer:1","urn:schemas-upnp-org:device:MediaServer:1","",upnp->NotifyCycleTime);
							ILibAsyncUDPSocket_SendTo(response_socket,mss->dest_addr.sin_addr.s_addr, ntohs(mss->dest_addr.sin_port), b, packetlength, ILibAsyncSocket_MemoryOwnership_USER);
						MediaServer_BuildSsdpResponsePacket(b,&packetlength,mss->localIPAddress,(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:ConnectionManager:1","urn:schemas-upnp-org:service:ConnectionManager:1","",upnp->NotifyCycleTime);
						ILibAsyncUDPSocket_SendTo(response_socket,mss->dest_addr.sin_addr.s_addr, ntohs(mss->dest_addr.sin_port), b, packetlength, ILibAsyncSocket_MemoryOwnership_USER);
						MediaServer_BuildSsdpResponsePacket(b,&packetlength,mss->localIPAddress,(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:ContentDirectory:1","urn:schemas-upnp-org:service:ContentDirectory:1","",upnp->NotifyCycleTime);
						ILibAsyncUDPSocket_SendTo(response_socket,mss->dest_addr.sin_addr.s_addr, ntohs(mss->dest_addr.sin_port), b, packetlength, ILibAsyncSocket_MemoryOwnership_USER);

   }
   if(STLength==(int)strlen(upnp->UUID) && memcmp(ST,upnp->UUID,(int)strlen(upnp->UUID))==0)
				{
						MediaServer_BuildSsdpResponsePacket(b,&packetlength,mss->localIPAddress,(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"",upnp->UUID,"",upnp->NotifyCycleTime);
						ILibAsyncUDPSocket_SendTo(response_socket,mss->dest_addr.sin_addr.s_addr, ntohs(mss->dest_addr.sin_port), b, packetlength, ILibAsyncSocket_MemoryOwnership_USER);
				}
				if(STLength>=40 && memcmp(ST,"urn:schemas-upnp-org:device:MediaServer:",40)==0 && atoi(ST+40)<=1)
				{
						MediaServer_BuildSsdpResponsePacket(b,&packetlength,mss->localIPAddress,(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:device:MediaServer:1",ST,"",upnp->NotifyCycleTime);
						ILibAsyncUDPSocket_SendTo(response_socket,mss->dest_addr.sin_addr.s_addr, ntohs(mss->dest_addr.sin_port), b, packetlength, ILibAsyncSocket_MemoryOwnership_USER);
				}
				if(STLength>=47 && memcmp(ST,"urn:schemas-upnp-org:service:ConnectionManager:",47)==0 && atoi(ST+47)<=1)
				{
						MediaServer_BuildSsdpResponsePacket(b,&packetlength,mss->localIPAddress,(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:ConnectionManager:1",ST,"",upnp->NotifyCycleTime);
						ILibAsyncUDPSocket_SendTo(response_socket,mss->dest_addr.sin_addr.s_addr, ntohs(mss->dest_addr.sin_port), b, packetlength, ILibAsyncSocket_MemoryOwnership_USER);
				}
				if(STLength>=46 && memcmp(ST,"urn:schemas-upnp-org:service:ContentDirectory:",46)==0 && atoi(ST+46)<=1)
				{
						MediaServer_BuildSsdpResponsePacket(b,&packetlength,mss->localIPAddress,(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:ContentDirectory:1",ST,"",upnp->NotifyCycleTime);
						ILibAsyncUDPSocket_SendTo(response_socket,mss->dest_addr.sin_addr.s_addr, ntohs(mss->dest_addr.sin_port), b, packetlength, ILibAsyncSocket_MemoryOwnership_USER);
				}

   
   ILibChain_DestroyEx(subChain);
   
   free(mss->ST);
   free(mss);
   free(b);
}
void MediaServer_ProcessMSEARCH(struct MediaServer_DataObject *upnp, struct packetheader *packet)
{
   char* ST = NULL;
   int STLength = 0;
   struct packetheader_field_node *node;
   int MANOK = 0;
   unsigned long MXVal;
   int MXOK = 0;
   int MX;
   struct MSEARCH_state *mss = NULL;
   
   if(memcmp(packet->DirectiveObj,"*",1)==0)
   {
      if(memcmp(packet->Version,"1.1",3)==0)
      {
         node = packet->FirstField;
         while(node!=NULL)
         {
            if(node->FieldLength==2 && strncasecmp(node->Field,"ST",2)==0)
            {
               //
               // This is what is being searched for
               //
               ST = (char*)malloc(1+node->FieldDataLength);
               memcpy(ST,node->FieldData,node->FieldDataLength);
               ST[node->FieldDataLength] = 0;
               STLength = node->FieldDataLength;
            }
            else if(node->FieldLength==3 && strncasecmp(node->Field,"MAN",3)==0 && memcmp(node->FieldData,"\"ssdp:discover\"",15)==0)
            {
               //
               // This is a required header field
               // 
               MANOK = 1;
            }
            else if(node->FieldLength==2 && strncasecmp(node->Field,"MX",2)==0 && ILibGetULong(node->FieldData,node->FieldDataLength,&MXVal)==0)
            {
               //
               // If the timeout value specified is greater than 10 seconds, just force it
               // down to 10 seconds
               //
               MXOK = 1;
               MXVal = MXVal>10?10:MXVal;
            }
            node = node->NextField;
         }
         if(MANOK!=0 && MXOK!=0)
         {
            if(MXVal==0)
            {
               MX = 0;
            }
            else
            {
               //
               // The timeout value should be a random number between 0 and the 
               // specified value
               //
               MX = (int)(0 + ((unsigned short)rand() % MXVal));
            }
            mss = (struct MSEARCH_state*)malloc(sizeof(struct MSEARCH_state));
            mss->ST = ST;
            mss->STLength = STLength;
            mss->upnp = upnp;
            memset((char *)&(mss->dest_addr), 0, sizeof(mss->dest_addr));
            mss->dest_addr.sin_family = AF_INET;
            mss->dest_addr.sin_addr = packet->Source->sin_addr;
            mss->dest_addr.sin_port = packet->Source->sin_port;
            mss->localIPAddress = packet->ReceivingAddress;
            
            //
            // Register for a timed callback, so we can respond later
            //
            ILibLifeTime_Add(upnp->WebServerTimer,mss,MX,&MediaServer_PostMX_MSEARCH,&MediaServer_PostMX_Destroy);
         }
         else
         {
            free(ST);
         }
      }
   }
}
#define MediaServer_Dispatch_ConnectionManager_GetCurrentConnectionIDs(buffer,offset,bufferLength, session)\
{\
	if(MediaServer_FP_ConnectionManager_GetCurrentConnectionIDs == NULL)\
		MediaServer_Response_Error(session,501,"No Function Handler");\
	else\
		MediaServer_FP_ConnectionManager_GetCurrentConnectionIDs((void*)session);\
}

void MediaServer_Dispatch_ConnectionManager_GetCurrentConnectionInfo(char *buffer, int offset, int bufferLength, struct ILibWebServer_Session *ReaderObject)
{
	long TempLong;
	int OK = 0;
	char *p_ConnectionID = NULL;
	int p_ConnectionIDLength = 0;
	int _ConnectionID = 0;
	struct ILibXMLNode *xnode = ILibParseXML(buffer,offset,bufferLength);
	struct ILibXMLNode *root = xnode;
	if(ILibProcessXMLNodeList(root)!=0)
	{
/* The XML is not well formed! */
      ILibDestructXMLNodeList(root);
	MediaServer_Response_Error(ReaderObject,501,"Invalid XML");
	return;
	}
	while(xnode!=NULL)
	{
		if(xnode->StartTag!=0 && xnode->NameLength==8 && memcmp(xnode->Name,"Envelope",8)==0)
		{
			// Envelope
			xnode = xnode->Next;
			while(xnode!=NULL)
			{
				if(xnode->StartTag!=0 && xnode->NameLength==4 && memcmp(xnode->Name,"Body",4)==0)
				{
					// Body
					xnode = xnode->Next;
					while(xnode!=NULL)
					{
						if(xnode->StartTag!=0 && xnode->NameLength==24 && memcmp(xnode->Name,"GetCurrentConnectionInfo",24)==0)
						{
							// Inside the interesting part of the SOAP
							xnode = xnode->Next;
							while(xnode!=NULL)
							{
								if(xnode->NameLength==12 && memcmp(xnode->Name,"ConnectionID",12)==0)
								{
									p_ConnectionIDLength = ILibReadInnerXML(xnode,&p_ConnectionID);
										OK |= 1;
								}
								if(xnode->Peer==NULL)
								{
									xnode = xnode->Parent;
									break;
								}
								else
								{
									xnode = xnode->Peer;
								}
							}
						}
						if(xnode->Peer==NULL)
						{
							xnode = xnode->Parent;
							break;
						}
						else
						{
							xnode = xnode->Peer;
						}
					}
				}
				if(xnode->Peer==NULL)
				{
					xnode = xnode->Parent;
					break;
				}
				else
				{
					xnode = xnode->Peer;
				}
			}
		}
		xnode = xnode->Peer;
	}
	ILibDestructXMLNodeList(root);
	if (OK != 1)
	{
		MediaServer_Response_Error(ReaderObject,402,"Illegal value");
		return;
	}

/* Type Checking */
   OK = ILibGetLong(p_ConnectionID,p_ConnectionIDLength, &TempLong);
	if(OK!=0)
	{
		MediaServer_Response_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_ConnectionID = (int)TempLong;
	if(MediaServer_FP_ConnectionManager_GetCurrentConnectionInfo == NULL)
		MediaServer_Response_Error(ReaderObject,501,"No Function Handler");
	else
		MediaServer_FP_ConnectionManager_GetCurrentConnectionInfo((void*)ReaderObject,_ConnectionID);
}

#define MediaServer_Dispatch_ConnectionManager_GetProtocolInfo(buffer,offset,bufferLength, session)\
{\
	if(MediaServer_FP_ConnectionManager_GetProtocolInfo == NULL)\
		MediaServer_Response_Error(session,501,"No Function Handler");\
	else\
		MediaServer_FP_ConnectionManager_GetProtocolInfo((void*)session);\
}

void MediaServer_Dispatch_ContentDirectory_Browse(char *buffer, int offset, int bufferLength, struct ILibWebServer_Session *ReaderObject)
{
	unsigned long TempULong;
	int OK = 0;
	char *p_ObjectID = NULL;
	int p_ObjectIDLength = 0;
	char* _ObjectID = "";
	int _ObjectIDLength;
	char *p_BrowseFlag = NULL;
	int p_BrowseFlagLength = 0;
	char* _BrowseFlag = "";
	int _BrowseFlagLength;
	char *p_Filter = NULL;
	int p_FilterLength = 0;
	char* _Filter = "";
	int _FilterLength;
	char *p_StartingIndex = NULL;
	int p_StartingIndexLength = 0;
	unsigned int _StartingIndex = 0;
	char *p_RequestedCount = NULL;
	int p_RequestedCountLength = 0;
	unsigned int _RequestedCount = 0;
	char *p_SortCriteria = NULL;
	int p_SortCriteriaLength = 0;
	char* _SortCriteria = "";
	int _SortCriteriaLength;
	struct ILibXMLNode *xnode = ILibParseXML(buffer,offset,bufferLength);
	struct ILibXMLNode *root = xnode;
	if(ILibProcessXMLNodeList(root)!=0)
	{
/* The XML is not well formed! */
      ILibDestructXMLNodeList(root);
	MediaServer_Response_Error(ReaderObject,501,"Invalid XML");
	return;
	}
	while(xnode!=NULL)
	{
		if(xnode->StartTag!=0 && xnode->NameLength==8 && memcmp(xnode->Name,"Envelope",8)==0)
		{
			// Envelope
			xnode = xnode->Next;
			while(xnode!=NULL)
			{
				if(xnode->StartTag!=0 && xnode->NameLength==4 && memcmp(xnode->Name,"Body",4)==0)
				{
					// Body
					xnode = xnode->Next;
					while(xnode!=NULL)
					{
						if(xnode->StartTag!=0 && xnode->NameLength==6 && memcmp(xnode->Name,"Browse",6)==0)
						{
							// Inside the interesting part of the SOAP
							xnode = xnode->Next;
							while(xnode!=NULL)
							{
								if(xnode->NameLength==8 && memcmp(xnode->Name,"ObjectID",8)==0)
								{
									p_ObjectIDLength = ILibReadInnerXML(xnode,&p_ObjectID);
									p_ObjectID[p_ObjectIDLength]=0;
										OK |= 1;
								}
								else if(xnode->NameLength==10 && memcmp(xnode->Name,"BrowseFlag",10)==0)
								{
									p_BrowseFlagLength = ILibReadInnerXML(xnode,&p_BrowseFlag);
									p_BrowseFlag[p_BrowseFlagLength]=0;
										OK |= 2;
								}
								else if(xnode->NameLength==6 && memcmp(xnode->Name,"Filter",6)==0)
								{
									p_FilterLength = ILibReadInnerXML(xnode,&p_Filter);
									p_Filter[p_FilterLength]=0;
										OK |= 4;
								}
								else if(xnode->NameLength==13 && memcmp(xnode->Name,"StartingIndex",13)==0)
								{
									p_StartingIndexLength = ILibReadInnerXML(xnode,&p_StartingIndex);
										OK |= 8;
								}
								else if(xnode->NameLength==14 && memcmp(xnode->Name,"RequestedCount",14)==0)
								{
									p_RequestedCountLength = ILibReadInnerXML(xnode,&p_RequestedCount);
										OK |= 16;
								}
								else if(xnode->NameLength==12 && memcmp(xnode->Name,"SortCriteria",12)==0)
								{
									p_SortCriteriaLength = ILibReadInnerXML(xnode,&p_SortCriteria);
									p_SortCriteria[p_SortCriteriaLength]=0;
										OK |= 32;
								}
								if(xnode->Peer==NULL)
								{
									xnode = xnode->Parent;
									break;
								}
								else
								{
									xnode = xnode->Peer;
								}
							}
						}
						if(xnode->Peer==NULL)
						{
							xnode = xnode->Parent;
							break;
						}
						else
						{
							xnode = xnode->Peer;
						}
					}
				}
				if(xnode->Peer==NULL)
				{
					xnode = xnode->Parent;
					break;
				}
				else
				{
					xnode = xnode->Peer;
				}
			}
		}
		xnode = xnode->Peer;
	}
	ILibDestructXMLNodeList(root);
	if (OK != 63)
	{
		MediaServer_Response_Error(ReaderObject,402,"Illegal value");
		return;
	}

/* Type Checking */
   _ObjectIDLength = ILibInPlaceXmlUnEscape(p_ObjectID);
	_ObjectID = p_ObjectID;
	_BrowseFlagLength = ILibInPlaceXmlUnEscape(p_BrowseFlag);
	_BrowseFlag = p_BrowseFlag;
	for(OK=0;OK<MediaServer__StateVariable_AllowedValues_MAX;++OK)
   {
      if(MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_BrowseFlag->AllowedValues[OK]!=NULL)
      {
         if(strcmp(_BrowseFlag,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_BrowseFlag->AllowedValues[OK])==0)
         {
            OK=0;
            break;
         }
      }
      else
      {
         break;
      }
   }
   if(OK!=0)
   {
		MediaServer_Response_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_FilterLength = ILibInPlaceXmlUnEscape(p_Filter);
	_Filter = p_Filter;
	OK = ILibGetULong(p_StartingIndex,p_StartingIndexLength, &TempULong);
	if(OK!=0)
	{
		MediaServer_Response_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_StartingIndex = (unsigned int)TempULong;
	OK = ILibGetULong(p_RequestedCount,p_RequestedCountLength, &TempULong);
	if(OK!=0)
	{
		MediaServer_Response_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_RequestedCount = (unsigned int)TempULong;
	_SortCriteriaLength = ILibInPlaceXmlUnEscape(p_SortCriteria);
	_SortCriteria = p_SortCriteria;
	if(MediaServer_FP_ContentDirectory_Browse == NULL)
		MediaServer_Response_Error(ReaderObject,501,"No Function Handler");
	else
		MediaServer_FP_ContentDirectory_Browse((void*)ReaderObject,_ObjectID,_BrowseFlag,_Filter,_StartingIndex,_RequestedCount,_SortCriteria);
}

void MediaServer_Dispatch_ContentDirectory_CreateObject(char *buffer, int offset, int bufferLength, struct ILibWebServer_Session *ReaderObject)
{
	int OK = 0;
	char *p_ContainerID = NULL;
	int p_ContainerIDLength = 0;
	char* _ContainerID = "";
	int _ContainerIDLength;
	char *p_Elements = NULL;
	int p_ElementsLength = 0;
	char* _Elements = "";
	int _ElementsLength;
	struct ILibXMLNode *xnode = ILibParseXML(buffer,offset,bufferLength);
	struct ILibXMLNode *root = xnode;
	if(ILibProcessXMLNodeList(root)!=0)
	{
/* The XML is not well formed! */
      ILibDestructXMLNodeList(root);
	MediaServer_Response_Error(ReaderObject,501,"Invalid XML");
	return;
	}
	while(xnode!=NULL)
	{
		if(xnode->StartTag!=0 && xnode->NameLength==8 && memcmp(xnode->Name,"Envelope",8)==0)
		{
			// Envelope
			xnode = xnode->Next;
			while(xnode!=NULL)
			{
				if(xnode->StartTag!=0 && xnode->NameLength==4 && memcmp(xnode->Name,"Body",4)==0)
				{
					// Body
					xnode = xnode->Next;
					while(xnode!=NULL)
					{
						if(xnode->StartTag!=0 && xnode->NameLength==12 && memcmp(xnode->Name,"CreateObject",12)==0)
						{
							// Inside the interesting part of the SOAP
							xnode = xnode->Next;
							while(xnode!=NULL)
							{
								if(xnode->NameLength==11 && memcmp(xnode->Name,"ContainerID",11)==0)
								{
									p_ContainerIDLength = ILibReadInnerXML(xnode,&p_ContainerID);
									p_ContainerID[p_ContainerIDLength]=0;
										OK |= 1;
								}
								else if(xnode->NameLength==8 && memcmp(xnode->Name,"Elements",8)==0)
								{
									p_ElementsLength = ILibReadInnerXML(xnode,&p_Elements);
									p_Elements[p_ElementsLength]=0;
										OK |= 2;
								}
								if(xnode->Peer==NULL)
								{
									xnode = xnode->Parent;
									break;
								}
								else
								{
									xnode = xnode->Peer;
								}
							}
						}
						if(xnode->Peer==NULL)
						{
							xnode = xnode->Parent;
							break;
						}
						else
						{
							xnode = xnode->Peer;
						}
					}
				}
				if(xnode->Peer==NULL)
				{
					xnode = xnode->Parent;
					break;
				}
				else
				{
					xnode = xnode->Peer;
				}
			}
		}
		xnode = xnode->Peer;
	}
	ILibDestructXMLNodeList(root);
	if (OK != 3)
	{
		MediaServer_Response_Error(ReaderObject,402,"Illegal value");
		return;
	}

/* Type Checking */
   _ContainerIDLength = ILibInPlaceXmlUnEscape(p_ContainerID);
	_ContainerID = p_ContainerID;
	_ElementsLength = ILibInPlaceXmlUnEscape(p_Elements);
	_Elements = p_Elements;
	if(MediaServer_FP_ContentDirectory_CreateObject == NULL)
		MediaServer_Response_Error(ReaderObject,501,"No Function Handler");
	else
		MediaServer_FP_ContentDirectory_CreateObject((void*)ReaderObject,_ContainerID,_Elements);
}

void MediaServer_Dispatch_ContentDirectory_DestroyObject(char *buffer, int offset, int bufferLength, struct ILibWebServer_Session *ReaderObject)
{
	int OK = 0;
	char *p_ObjectID = NULL;
	int p_ObjectIDLength = 0;
	char* _ObjectID = "";
	int _ObjectIDLength;
	struct ILibXMLNode *xnode = ILibParseXML(buffer,offset,bufferLength);
	struct ILibXMLNode *root = xnode;
	if(ILibProcessXMLNodeList(root)!=0)
	{
/* The XML is not well formed! */
      ILibDestructXMLNodeList(root);
	MediaServer_Response_Error(ReaderObject,501,"Invalid XML");
	return;
	}
	while(xnode!=NULL)
	{
		if(xnode->StartTag!=0 && xnode->NameLength==8 && memcmp(xnode->Name,"Envelope",8)==0)
		{
			// Envelope
			xnode = xnode->Next;
			while(xnode!=NULL)
			{
				if(xnode->StartTag!=0 && xnode->NameLength==4 && memcmp(xnode->Name,"Body",4)==0)
				{
					// Body
					xnode = xnode->Next;
					while(xnode!=NULL)
					{
						if(xnode->StartTag!=0 && xnode->NameLength==13 && memcmp(xnode->Name,"DestroyObject",13)==0)
						{
							// Inside the interesting part of the SOAP
							xnode = xnode->Next;
							while(xnode!=NULL)
							{
								if(xnode->NameLength==8 && memcmp(xnode->Name,"ObjectID",8)==0)
								{
									p_ObjectIDLength = ILibReadInnerXML(xnode,&p_ObjectID);
									p_ObjectID[p_ObjectIDLength]=0;
										OK |= 1;
								}
								if(xnode->Peer==NULL)
								{
									xnode = xnode->Parent;
									break;
								}
								else
								{
									xnode = xnode->Peer;
								}
							}
						}
						if(xnode->Peer==NULL)
						{
							xnode = xnode->Parent;
							break;
						}
						else
						{
							xnode = xnode->Peer;
						}
					}
				}
				if(xnode->Peer==NULL)
				{
					xnode = xnode->Parent;
					break;
				}
				else
				{
					xnode = xnode->Peer;
				}
			}
		}
		xnode = xnode->Peer;
	}
	ILibDestructXMLNodeList(root);
	if (OK != 1)
	{
		MediaServer_Response_Error(ReaderObject,402,"Illegal value");
		return;
	}

/* Type Checking */
   _ObjectIDLength = ILibInPlaceXmlUnEscape(p_ObjectID);
	_ObjectID = p_ObjectID;
	if(MediaServer_FP_ContentDirectory_DestroyObject == NULL)
		MediaServer_Response_Error(ReaderObject,501,"No Function Handler");
	else
		MediaServer_FP_ContentDirectory_DestroyObject((void*)ReaderObject,_ObjectID);
}

#define MediaServer_Dispatch_ContentDirectory_GetSearchCapabilities(buffer,offset,bufferLength, session)\
{\
	if(MediaServer_FP_ContentDirectory_GetSearchCapabilities == NULL)\
		MediaServer_Response_Error(session,501,"No Function Handler");\
	else\
		MediaServer_FP_ContentDirectory_GetSearchCapabilities((void*)session);\
}

#define MediaServer_Dispatch_ContentDirectory_GetSortCapabilities(buffer,offset,bufferLength, session)\
{\
	if(MediaServer_FP_ContentDirectory_GetSortCapabilities == NULL)\
		MediaServer_Response_Error(session,501,"No Function Handler");\
	else\
		MediaServer_FP_ContentDirectory_GetSortCapabilities((void*)session);\
}

#define MediaServer_Dispatch_ContentDirectory_GetSystemUpdateID(buffer,offset,bufferLength, session)\
{\
	if(MediaServer_FP_ContentDirectory_GetSystemUpdateID == NULL)\
		MediaServer_Response_Error(session,501,"No Function Handler");\
	else\
		MediaServer_FP_ContentDirectory_GetSystemUpdateID((void*)session);\
}

void MediaServer_Dispatch_ContentDirectory_Search(char *buffer, int offset, int bufferLength, struct ILibWebServer_Session *ReaderObject)
{
	unsigned long TempULong;
	int OK = 0;
	char *p_ContainerID = NULL;
	int p_ContainerIDLength = 0;
	char* _ContainerID = "";
	int _ContainerIDLength;
	char *p_SearchCriteria = NULL;
	int p_SearchCriteriaLength = 0;
	char* _SearchCriteria = "";
	int _SearchCriteriaLength;
	char *p_Filter = NULL;
	int p_FilterLength = 0;
	char* _Filter = "";
	int _FilterLength;
	char *p_StartingIndex = NULL;
	int p_StartingIndexLength = 0;
	unsigned int _StartingIndex = 0;
	char *p_RequestedCount = NULL;
	int p_RequestedCountLength = 0;
	unsigned int _RequestedCount = 0;
	char *p_SortCriteria = NULL;
	int p_SortCriteriaLength = 0;
	char* _SortCriteria = "";
	int _SortCriteriaLength;
	struct ILibXMLNode *xnode = ILibParseXML(buffer,offset,bufferLength);
	struct ILibXMLNode *root = xnode;
	if(ILibProcessXMLNodeList(root)!=0)
	{
/* The XML is not well formed! */
      ILibDestructXMLNodeList(root);
	MediaServer_Response_Error(ReaderObject,501,"Invalid XML");
	return;
	}
	while(xnode!=NULL)
	{
		if(xnode->StartTag!=0 && xnode->NameLength==8 && memcmp(xnode->Name,"Envelope",8)==0)
		{
			// Envelope
			xnode = xnode->Next;
			while(xnode!=NULL)
			{
				if(xnode->StartTag!=0 && xnode->NameLength==4 && memcmp(xnode->Name,"Body",4)==0)
				{
					// Body
					xnode = xnode->Next;
					while(xnode!=NULL)
					{
						if(xnode->StartTag!=0 && xnode->NameLength==6 && memcmp(xnode->Name,"Search",6)==0)
						{
							// Inside the interesting part of the SOAP
							xnode = xnode->Next;
							while(xnode!=NULL)
							{
								if(xnode->NameLength==11 && memcmp(xnode->Name,"ContainerID",11)==0)
								{
									p_ContainerIDLength = ILibReadInnerXML(xnode,&p_ContainerID);
									p_ContainerID[p_ContainerIDLength]=0;
										OK |= 1;
								}
								else if(xnode->NameLength==14 && memcmp(xnode->Name,"SearchCriteria",14)==0)
								{
									p_SearchCriteriaLength = ILibReadInnerXML(xnode,&p_SearchCriteria);
									p_SearchCriteria[p_SearchCriteriaLength]=0;
										OK |= 2;
								}
								else if(xnode->NameLength==6 && memcmp(xnode->Name,"Filter",6)==0)
								{
									p_FilterLength = ILibReadInnerXML(xnode,&p_Filter);
									p_Filter[p_FilterLength]=0;
										OK |= 4;
								}
								else if(xnode->NameLength==13 && memcmp(xnode->Name,"StartingIndex",13)==0)
								{
									p_StartingIndexLength = ILibReadInnerXML(xnode,&p_StartingIndex);
										OK |= 8;
								}
								else if(xnode->NameLength==14 && memcmp(xnode->Name,"RequestedCount",14)==0)
								{
									p_RequestedCountLength = ILibReadInnerXML(xnode,&p_RequestedCount);
										OK |= 16;
								}
								else if(xnode->NameLength==12 && memcmp(xnode->Name,"SortCriteria",12)==0)
								{
									p_SortCriteriaLength = ILibReadInnerXML(xnode,&p_SortCriteria);
									p_SortCriteria[p_SortCriteriaLength]=0;
										OK |= 32;
								}
								if(xnode->Peer==NULL)
								{
									xnode = xnode->Parent;
									break;
								}
								else
								{
									xnode = xnode->Peer;
								}
							}
						}
						if(xnode->Peer==NULL)
						{
							xnode = xnode->Parent;
							break;
						}
						else
						{
							xnode = xnode->Peer;
						}
					}
				}
				if(xnode->Peer==NULL)
				{
					xnode = xnode->Parent;
					break;
				}
				else
				{
					xnode = xnode->Peer;
				}
			}
		}
		xnode = xnode->Peer;
	}
	ILibDestructXMLNodeList(root);
	if (OK != 63)
	{
		MediaServer_Response_Error(ReaderObject,402,"Illegal value");
		return;
	}

/* Type Checking */
   _ContainerIDLength = ILibInPlaceXmlUnEscape(p_ContainerID);
	_ContainerID = p_ContainerID;
	_SearchCriteriaLength = ILibInPlaceXmlUnEscape(p_SearchCriteria);
	_SearchCriteria = p_SearchCriteria;
	_FilterLength = ILibInPlaceXmlUnEscape(p_Filter);
	_Filter = p_Filter;
	OK = ILibGetULong(p_StartingIndex,p_StartingIndexLength, &TempULong);
	if(OK!=0)
	{
		MediaServer_Response_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_StartingIndex = (unsigned int)TempULong;
	OK = ILibGetULong(p_RequestedCount,p_RequestedCountLength, &TempULong);
	if(OK!=0)
	{
		MediaServer_Response_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_RequestedCount = (unsigned int)TempULong;
	_SortCriteriaLength = ILibInPlaceXmlUnEscape(p_SortCriteria);
	_SortCriteria = p_SortCriteria;
	if(MediaServer_FP_ContentDirectory_Search == NULL)
		MediaServer_Response_Error(ReaderObject,501,"No Function Handler");
	else
		MediaServer_FP_ContentDirectory_Search((void*)ReaderObject,_ContainerID,_SearchCriteria,_Filter,_StartingIndex,_RequestedCount,_SortCriteria);
}


int MediaServer_ProcessPOST(struct ILibWebServer_Session *session, struct packetheader* header, char *bodyBuffer, int offset, int bodyBufferLength)
{
   struct packetheader_field_node *f = header->FirstField;
   char* HOST;
   char* SOAPACTION = NULL;
   int SOAPACTIONLength = 0;
   struct parser_result *r,*r2;
   struct parser_result_field *prf;
   
   int RetVal = 0;
   
   //
   // Iterate through all the HTTP Headers
   //
   while(f!=NULL)
   {
      if(f->FieldLength==4 && strncasecmp(f->Field,"HOST",4)==0)
      {
         HOST = f->FieldData;
      }
      else if(f->FieldLength==10 && strncasecmp(f->Field,"SOAPACTION",10)==0)
      {
         r = ILibParseString(f->FieldData,0,f->FieldDataLength,"#",1);
         SOAPACTION = r->LastResult->data;
         SOAPACTIONLength = r->LastResult->datalength-1;
         ILibDestructParserResults(r);
      }
      else if(f->FieldLength==10 && strncasecmp(f->Field,"USER-AGENT",10)==0)
      {
         // Check UPnP version of the Control Point which invoked us
         r = ILibParseString(f->FieldData,0,f->FieldDataLength," ",1);
         prf = r->FirstResult;
         while(prf!=NULL)
         {
            if(prf->datalength>5 && memcmp(prf->data,"UPnP/",5)==0)
            {
               r2 = ILibParseString(prf->data+5,0,prf->datalength-5,".",1);
               r2->FirstResult->data[r2->FirstResult->datalength]=0;
               r2->LastResult->data[r2->LastResult->datalength]=0;
               if(atoi(r2->FirstResult->data)==1 && atoi(r2->LastResult->data)>0)
               {
                  session->Reserved9=1;
               }
               ILibDestructParserResults(r2);
            }
            prf = prf->NextResult;
         }
         ILibDestructParserResults(r);
      }
      f = f->NextField;
   }
   
   if(header->DirectiveObjLength==26 && memcmp((header->DirectiveObj)+1,"ConnectionManager/control",25)==0)
	{
		 if(SOAPACTIONLength==23 && memcmp(SOAPACTION,"GetCurrentConnectionIDs",23)==0)
		{
			MediaServer_Dispatch_ConnectionManager_GetCurrentConnectionIDs(bodyBuffer, offset, bodyBufferLength, session);
		}
		else if(SOAPACTIONLength==24 && memcmp(SOAPACTION,"GetCurrentConnectionInfo",24)==0)
		{
			MediaServer_Dispatch_ConnectionManager_GetCurrentConnectionInfo(bodyBuffer, offset, bodyBufferLength, session);
		}
		else if(SOAPACTIONLength==15 && memcmp(SOAPACTION,"GetProtocolInfo",15)==0)
		{
			MediaServer_Dispatch_ConnectionManager_GetProtocolInfo(bodyBuffer, offset, bodyBufferLength, session);
		}
		else
		{
			RetVal=1;
		}
	}
	else if(header->DirectiveObjLength==25 && memcmp((header->DirectiveObj)+1,"ContentDirectory/control",24)==0)
	{
		 if(SOAPACTIONLength==6 && memcmp(SOAPACTION,"Browse",6)==0)
		{
			MediaServer_Dispatch_ContentDirectory_Browse(bodyBuffer, offset, bodyBufferLength, session);
		}
		else if(SOAPACTIONLength==12 && memcmp(SOAPACTION,"CreateObject",12)==0)
		{
			MediaServer_Dispatch_ContentDirectory_CreateObject(bodyBuffer, offset, bodyBufferLength, session);
		}
		else if(SOAPACTIONLength==13 && memcmp(SOAPACTION,"DestroyObject",13)==0)
		{
			MediaServer_Dispatch_ContentDirectory_DestroyObject(bodyBuffer, offset, bodyBufferLength, session);
		}
		else if(SOAPACTIONLength==21 && memcmp(SOAPACTION,"GetSearchCapabilities",21)==0)
		{
			MediaServer_Dispatch_ContentDirectory_GetSearchCapabilities(bodyBuffer, offset, bodyBufferLength, session);
		}
		else if(SOAPACTIONLength==19 && memcmp(SOAPACTION,"GetSortCapabilities",19)==0)
		{
			MediaServer_Dispatch_ContentDirectory_GetSortCapabilities(bodyBuffer, offset, bodyBufferLength, session);
		}
		else if(SOAPACTIONLength==17 && memcmp(SOAPACTION,"GetSystemUpdateID",17)==0)
		{
			MediaServer_Dispatch_ContentDirectory_GetSystemUpdateID(bodyBuffer, offset, bodyBufferLength, session);
		}
		else if(SOAPACTIONLength==6 && memcmp(SOAPACTION,"Search",6)==0)
		{
			MediaServer_Dispatch_ContentDirectory_Search(bodyBuffer, offset, bodyBufferLength, session);
		}
		else
		{
			RetVal=1;
		}
	}
	else
	{
		RetVal=1;
	}

   
   return(RetVal);
}

struct SubscriberInfo* MediaServer_RemoveSubscriberInfo(struct SubscriberInfo **Head, int *TotalSubscribers,char* SID, int SIDLength)
{
   struct SubscriberInfo *info = *Head;
   while(info!=NULL)
   {
      if(info->SIDLength==SIDLength && memcmp(info->SID,SID,SIDLength)==0)
      {
         if ( info->Previous )
         info->Previous->Next = info->Next;
         else
         *Head = info->Next;
         if ( info->Next )
         info->Next->Previous = info->Previous;
         break;
      }
      info = info->Next;
      
   }
   if(info!=NULL)
   {
      info->Previous = NULL;
      info->Next = NULL;
      --(*TotalSubscribers);
   }
   return(info);
}

#define MediaServer_DestructSubscriberInfo(info)\
{\
   free(info->Path);\
   free(info->SID);\
   free(info);\
}

#define MediaServer_DestructEventObject(EvObject)\
{\
   free(EvObject->PacketBody);\
   free(EvObject);\
}

#define MediaServer_DestructEventDataObject(EvData)\
{\
   free(EvData);\
}
void MediaServer_ExpireSubscriberInfo(struct MediaServer_DataObject *d, struct SubscriberInfo *info)
{
   struct SubscriberInfo *t = info;
   while(t->Previous!=NULL)
   {
      t = t->Previous;
   }
   if(d->HeadSubscriberPtr_ConnectionManager==t)
	{
		--(d->NumberOfSubscribers_ConnectionManager);
	}
	else if(d->HeadSubscriberPtr_ContentDirectory==t)
	{
		--(d->NumberOfSubscribers_ContentDirectory);
	}

   
   if(info->Previous!=NULL)
   {
      // This is not the Head
      info->Previous->Next = info->Next;
      if(info->Next!=NULL)
      {
         info->Next->Previous = info->Previous;
      }
   }
   else
   {
      // This is the Head
      if(d->HeadSubscriberPtr_ConnectionManager==info)
	{
		d->HeadSubscriberPtr_ConnectionManager = info->Next;
		if(info->Next!=NULL)
		{
			info->Next->Previous = NULL;
		}
	}
	else if(d->HeadSubscriberPtr_ContentDirectory==info)
	{
		d->HeadSubscriberPtr_ContentDirectory = info->Next;
		if(info->Next!=NULL)
		{
			info->Next->Previous = NULL;
		}
	}
	else 
	{
		// Error
		return;
	}

   }
   --info->RefCount;
   if(info->RefCount==0)
   {
      MediaServer_DestructSubscriberInfo(info);
   }
}

int MediaServer_SubscriptionExpired(struct SubscriberInfo *info)
{
   int RetVal = 0;
   
   struct timeval tv;
   gettimeofday(&tv,NULL);
   if((info->RenewByTime).tv_sec < tv.tv_sec) {RetVal = -1;}
   
   return(RetVal);
}

void MediaServer_GetInitialEventBody_ConnectionManager(struct MediaServer_DataObject *MediaServer_Object,char ** body, int *bodylength)
{
	int TempLength;
	TempLength = (int)(177+(int)strlen(MediaServer_Object->ConnectionManager_SourceProtocolInfo)+(int)strlen(MediaServer_Object->ConnectionManager_SinkProtocolInfo)+(int)strlen(MediaServer_Object->ConnectionManager_CurrentConnectionIDs));
	*body = (char*)malloc(sizeof(char)*TempLength);
	*bodylength = sprintf(*body,"SourceProtocolInfo>%s</SourceProtocolInfo></e:property><e:property><SinkProtocolInfo>%s</SinkProtocolInfo></e:property><e:property><CurrentConnectionIDs>%s</CurrentConnectionIDs",MediaServer_Object->ConnectionManager_SourceProtocolInfo,MediaServer_Object->ConnectionManager_SinkProtocolInfo,MediaServer_Object->ConnectionManager_CurrentConnectionIDs);
}
void MediaServer_GetInitialEventBody_ContentDirectory(struct MediaServer_DataObject *MediaServer_Object,char ** body, int *bodylength)
{
	int TempLength;
	TempLength = (int)(101+(int)strlen(MediaServer_Object->ContentDirectory_ContainerUpdateIDs)+(int)strlen(MediaServer_Object->ContentDirectory_SystemUpdateID));
	*body = (char*)malloc(sizeof(char)*TempLength);
	*bodylength = sprintf(*body,"ContainerUpdateIDs>%s</ContainerUpdateIDs></e:property><e:property><SystemUpdateID>%s</SystemUpdateID",MediaServer_Object->ContentDirectory_ContainerUpdateIDs,MediaServer_Object->ContentDirectory_SystemUpdateID);
}


void MediaServer_ProcessUNSUBSCRIBE(struct packetheader *header, struct ILibWebServer_Session *session)
{
   char* SID = NULL;
   int SIDLength = 0;
   struct SubscriberInfo *Info;
   struct packetheader_field_node *f;
   char* packet = (char*)malloc(sizeof(char)*50);
   int packetlength;
   
   //
   // Iterate through all the HTTP headers
   //
   f = header->FirstField;
   while(f!=NULL)
   {
      if(f->FieldLength==3)
      {
         if(strncasecmp(f->Field,"SID",3)==0)
         {
            //
            // Get the Subscription ID
            //
            SID = f->FieldData;
            SIDLength = f->FieldDataLength;
         }
      }
      f = f->NextField;
   }
   sem_wait(&(((struct MediaServer_DataObject*)session->User)->EventLock));
   if(header->DirectiveObjLength==24 && memcmp(header->DirectiveObj + 1,"ConnectionManager/event",23)==0)
	{
		Info = MediaServer_RemoveSubscriberInfo(&(((struct MediaServer_DataObject*)session->User)->HeadSubscriberPtr_ConnectionManager),&(((struct MediaServer_DataObject*)session->User)->NumberOfSubscribers_ConnectionManager),SID,SIDLength);
		if(Info!=NULL)
		{
			--Info->RefCount;
			if(Info->RefCount==0)
			{
				MediaServer_DestructSubscriberInfo(Info);
			}
			packetlength = sprintf(packet,"HTTP/1.1 %d %s\r\nContent-Length: 0\r\n\r\n",200,"OK");
			ILibWebServer_Send_Raw(session,packet,packetlength,0,1);
		}
		else
		{
			packetlength = sprintf(packet,"HTTP/1.1 %d %s\r\nContent-Length: 0\r\n\r\n",412,"Invalid SID");
			ILibWebServer_Send_Raw(session,packet,packetlength,0,1);
		}
	}
	else if(header->DirectiveObjLength==23 && memcmp(header->DirectiveObj + 1,"ContentDirectory/event",22)==0)
	{
		Info = MediaServer_RemoveSubscriberInfo(&(((struct MediaServer_DataObject*)session->User)->HeadSubscriberPtr_ContentDirectory),&(((struct MediaServer_DataObject*)session->User)->NumberOfSubscribers_ContentDirectory),SID,SIDLength);
		if(Info!=NULL)
		{
			--Info->RefCount;
			if(Info->RefCount==0)
			{
				MediaServer_DestructSubscriberInfo(Info);
			}
			packetlength = sprintf(packet,"HTTP/1.1 %d %s\r\nContent-Length: 0\r\n\r\n",200,"OK");
			ILibWebServer_Send_Raw(session,packet,packetlength,0,1);
		}
		else
		{
			packetlength = sprintf(packet,"HTTP/1.1 %d %s\r\nContent-Length: 0\r\n\r\n",412,"Invalid SID");
			ILibWebServer_Send_Raw(session,packet,packetlength,0,1);
		}
	}

   sem_post(&(((struct MediaServer_DataObject*)session->User)->EventLock));
}
void MediaServer_TryToSubscribe(char* ServiceName, long Timeout, char* URL, int URLLength,struct ILibWebServer_Session *session)
{
   int *TotalSubscribers = NULL;
   struct SubscriberInfo **HeadPtr = NULL;
   struct SubscriberInfo *NewSubscriber,*TempSubscriber;
   int SIDNumber,rnumber;
   char *SID;
   char *TempString;
   int TempStringLength;
   char *TempString2;
   long TempLong;
   char *packet;
   int packetlength;
   char* path;
   
   char* escapedURI;
   int escapedURILength;
   
   char *packetbody = NULL;
   int packetbodyLength;
   
   struct parser_result *p;
   struct parser_result *p2;
   
   struct MediaServer_DataObject *dataObject = (struct MediaServer_DataObject*)session->User;
   
   if(strncmp(ServiceName,"ConnectionManager",17)==0)
	{
		TotalSubscribers = &(dataObject->NumberOfSubscribers_ConnectionManager);
		HeadPtr = &(dataObject->HeadSubscriberPtr_ConnectionManager);
	}
	if(strncmp(ServiceName,"ContentDirectory",16)==0)
	{
		TotalSubscribers = &(dataObject->NumberOfSubscribers_ContentDirectory);
		HeadPtr = &(dataObject->HeadSubscriberPtr_ContentDirectory);
	}

   
   if(*HeadPtr!=NULL)
   {
      NewSubscriber = *HeadPtr;
      while(NewSubscriber!=NULL)
      {
         if(MediaServer_SubscriptionExpired(NewSubscriber)!=0)
         {
            TempSubscriber = NewSubscriber->Next;
            NewSubscriber = MediaServer_RemoveSubscriberInfo(HeadPtr,TotalSubscribers,NewSubscriber->SID,NewSubscriber->SIDLength);
            MediaServer_DestructSubscriberInfo(NewSubscriber);
            NewSubscriber = TempSubscriber;
         }
         else
         {
            NewSubscriber = NewSubscriber->Next;
         }
      }
   }
   //
   // The Maximum number of subscribers can be bounded
   //
   if(*TotalSubscribers<10)
   {
      NewSubscriber = (struct SubscriberInfo*)malloc(sizeof(struct SubscriberInfo));
      memset(NewSubscriber,0,sizeof(struct SubscriberInfo));
      
      
      //
      // The SID must be globally unique, so lets generate it using
      // a bunch of random hex characters
      //
      SID = (char*)malloc(43);
      memset(SID,0,38);
      sprintf(SID,"uuid:");
      for(SIDNumber=5;SIDNumber<=12;++SIDNumber)
      {
         rnumber = rand()%16;
         sprintf(SID+SIDNumber,"%x",rnumber);
      }
      sprintf(SID+SIDNumber,"-");
      for(SIDNumber=14;SIDNumber<=17;++SIDNumber)
      {
         rnumber = rand()%16;
         sprintf(SID+SIDNumber,"%x",rnumber);
      }
      sprintf(SID+SIDNumber,"-");
      for(SIDNumber=19;SIDNumber<=22;++SIDNumber)
      {
         rnumber = rand()%16;
         sprintf(SID+SIDNumber,"%x",rnumber);
      }
      sprintf(SID+SIDNumber,"-");
      for(SIDNumber=24;SIDNumber<=27;++SIDNumber)
      {
         rnumber = rand()%16;
         sprintf(SID+SIDNumber,"%x",rnumber);
      }
      sprintf(SID+SIDNumber,"-");
      for(SIDNumber=29;SIDNumber<=40;++SIDNumber)
      {
         rnumber = rand()%16;
         sprintf(SID+SIDNumber,"%x",rnumber);
      }
      
      p = ILibParseString(URL,0,URLLength,"://",3);
      if(p->NumResults==1)
      {
         ILibWebServer_Send_Raw(session,"HTTP/1.1 412 Precondition Failed\r\nContent-Length: 0\r\n\r\n",55,1,1);
         ILibDestructParserResults(p);
         return;
      }
      TempString = p->LastResult->data;
      TempStringLength = p->LastResult->datalength;
      ILibDestructParserResults(p);
      p = ILibParseString(TempString,0,TempStringLength,"/",1);
      p2 = ILibParseString(p->FirstResult->data,0,p->FirstResult->datalength,":",1);
      TempString2 = (char*)malloc(1+sizeof(char)*p2->FirstResult->datalength);
      memcpy(TempString2,p2->FirstResult->data,p2->FirstResult->datalength);
      TempString2[p2->FirstResult->datalength] = '\0';
      NewSubscriber->Address = inet_addr(TempString2);
      if(p2->NumResults==1)
      {
         NewSubscriber->Port = 80;
         path = (char*)malloc(1+TempStringLength - p2->FirstResult->datalength -1);
         memcpy(path,TempString + p2->FirstResult->datalength,TempStringLength - p2->FirstResult->datalength -1);
         path[TempStringLength - p2->FirstResult->datalength - 1] = '\0';
         NewSubscriber->Path = path;
         NewSubscriber->PathLength = (int)strlen(path);
      }
      else
      {
         ILibGetLong(p2->LastResult->data,p2->LastResult->datalength,&TempLong);
         NewSubscriber->Port = (unsigned short)TempLong;
         if(TempStringLength==p->FirstResult->datalength)
         {
            path = (char*)malloc(2);
            memcpy(path,"/",1);
            path[1] = '\0';
         }
         else
         {
            path = (char*)malloc(1+TempStringLength - p->FirstResult->datalength -1);
            memcpy(path,TempString + p->FirstResult->datalength,TempStringLength - p->FirstResult->datalength -1);
            path[TempStringLength - p->FirstResult->datalength -1] = '\0';
         }
         NewSubscriber->Path = path;
         NewSubscriber->PathLength = (int)strlen(path);
      }
      ILibDestructParserResults(p);
      ILibDestructParserResults(p2);
      free(TempString2);
      
      
      escapedURI = (char*)malloc(ILibHTTPEscapeLength(NewSubscriber->Path));
      escapedURILength = ILibHTTPEscape(escapedURI,NewSubscriber->Path);
      
      free(NewSubscriber->Path);
      NewSubscriber->Path = escapedURI;
      NewSubscriber->PathLength = escapedURILength;
      
      
      NewSubscriber->RefCount = 1;
      NewSubscriber->Disposing = 0;
      NewSubscriber->Previous = NULL;
      NewSubscriber->SID = SID;
      NewSubscriber->SIDLength = (int)strlen(SID);
      NewSubscriber->SEQ = 0;
      
      //
      // Determine what the subscription renewal cycle is
      //
      
      gettimeofday(&(NewSubscriber->RenewByTime),NULL);
      (NewSubscriber->RenewByTime).tv_sec += (int)Timeout;
      
      NewSubscriber->Next = *HeadPtr;
      if(*HeadPtr!=NULL) {(*HeadPtr)->Previous = NewSubscriber;}
      *HeadPtr = NewSubscriber;
      ++(*TotalSubscribers);
      LVL3DEBUG(printf("\r\n\r\nSubscribed [%s] %d.%d.%d.%d:%d FOR %d Duration\r\n",NewSubscriber->SID,(NewSubscriber->Address)&0xFF,(NewSubscriber->Address>>8)&0xFF,(NewSubscriber->Address>>16)&0xFF,(NewSubscriber->Address>>24)&0xFF,NewSubscriber->Port,Timeout);)
      
      LVL3DEBUG(printf("TIMESTAMP: %d <%d>\r\n\r\n",(NewSubscriber->RenewByTime).tv_sec-Timeout,NewSubscriber);)
      
      packet = (char*)malloc(134 + (int)strlen(SID) + (int)strlen(MediaServer_PLATFORM) + 4);
      packetlength = sprintf(packet,"HTTP/1.1 200 OK\r\nSERVER: %s, UPnP/1.0, Intel MicroStack/1.0.2384\r\nSID: %s\r\nTIMEOUT: Second-%ld\r\nContent-Length: 0\r\n\r\n",MediaServer_PLATFORM,SID,Timeout);
      if(strcmp(ServiceName,"ConnectionManager")==0)
	{
		MediaServer_GetInitialEventBody_ConnectionManager(dataObject,&packetbody,&packetbodyLength);
	}
	else if(strcmp(ServiceName,"ContentDirectory")==0)
	{
		MediaServer_GetInitialEventBody_ContentDirectory(dataObject,&packetbody,&packetbodyLength);
	}

      if (packetbody != NULL)	    {
         ILibWebServer_Send_Raw(session,packet,packetlength,0,1);
         
         MediaServer_SendEvent_Body(dataObject,packetbody,packetbodyLength,NewSubscriber);
         free(packetbody);
      } 
   }
   else
   {
      /* Too many subscribers */
      ILibWebServer_Send_Raw(session,"HTTP/1.1 412 Too Many Subscribers\r\nContent-Length: 0\r\n\r\n",56,1,1);
   }
}

void MediaServer_SubscribeEvents(char* path,int pathlength,char* Timeout,int TimeoutLength,char* URL,int URLLength,struct ILibWebServer_Session* session)
{
   long TimeoutVal;
   char* buffer = (char*)malloc(1+sizeof(char)*pathlength);
   
   ILibGetLong(Timeout,TimeoutLength,&TimeoutVal);
   memcpy(buffer,path,pathlength);
   buffer[pathlength] = '\0';
   free(buffer);
   if(TimeoutVal>MediaServer__MAX_SUBSCRIPTION_TIMEOUT) {TimeoutVal=MediaServer__MAX_SUBSCRIPTION_TIMEOUT;}
   
   if(pathlength==24 && memcmp(path+1,"ConnectionManager/event",23)==0)
	{
		MediaServer_TryToSubscribe("ConnectionManager",TimeoutVal,URL,URLLength,session);
	}
else if(pathlength==23 && memcmp(path+1,"ContentDirectory/event",22)==0)
	{
		MediaServer_TryToSubscribe("ContentDirectory",TimeoutVal,URL,URLLength,session);
	}
	else
	{
		ILibWebServer_Send_Raw(session,"HTTP/1.1 412 Invalid Service Name\r\nContent-Length: 0\r\n\r\n",56,1,1);
	}

}

void MediaServer_RenewEvents(char* path,int pathlength,char *_SID,int SIDLength, char* Timeout, int TimeoutLength, struct ILibWebServer_Session *ReaderObject)
{
   struct SubscriberInfo *info = NULL;
   long TimeoutVal;
   
   struct timeval tv;
   
   char* packet;
   int packetlength;
   char* SID = (char*)malloc(SIDLength+1);
   memcpy(SID,_SID,SIDLength);
   SID[SIDLength] ='\0';
   
   LVL3DEBUG(gettimeofday(&tv,NULL);)
   LVL3DEBUG(printf("\r\n\r\nTIMESTAMP: %d\r\n",tv.tv_sec);)
   
   LVL3DEBUG(printf("SUBSCRIBER [%s] attempting to Renew Events for %s Duration [",SID,Timeout);)
   
   if(pathlength==24 && memcmp(path+1,"ConnectionManager/event",23)==0)
	{
		info = ((struct MediaServer_DataObject*)ReaderObject->User)->HeadSubscriberPtr_ConnectionManager;
	}
else if(pathlength==23 && memcmp(path+1,"ContentDirectory/event",22)==0)
	{
		info = ((struct MediaServer_DataObject*)ReaderObject->User)->HeadSubscriberPtr_ContentDirectory;
	}

   
   //
   // Find this SID in the subscriber list, and recalculate
   // the expiration timeout
   //
   while(info!=NULL && strcmp(info->SID,SID)!=0)
   {
      info = info->Next;
   }
   if(info!=NULL)
   {
      ILibGetLong(Timeout,TimeoutLength,&TimeoutVal);
      if(TimeoutVal>MediaServer__MAX_SUBSCRIPTION_TIMEOUT) {TimeoutVal=MediaServer__MAX_SUBSCRIPTION_TIMEOUT;}
      
      gettimeofday(&tv,NULL);
      (info->RenewByTime).tv_sec = tv.tv_sec + TimeoutVal;
      
      packet = (char*)malloc(134 + (int)strlen(SID) + 4);
      packetlength = sprintf(packet,"HTTP/1.1 200 OK\r\nSERVER: %s, UPnP/1.0, Intel MicroStack/1.0.2384\r\nSID: %s\r\nTIMEOUT: Second-%ld\r\nContent-Length: 0\r\n\r\n",MediaServer_PLATFORM,SID,TimeoutVal);
      ILibWebServer_Send_Raw(ReaderObject,packet,packetlength,0,1);
      LVL3DEBUG(printf("OK] {%d} <%d>\r\n\r\n",TimeoutVal,info);)
   }
   else
   {
      LVL3DEBUG(printf("FAILED]\r\n\r\n");)
      ILibWebServer_Send_Raw(ReaderObject,"HTTP/1.1 412 Precondition Failed\r\nContent-Length: 0\r\n\r\n",55,1,1);
   }
   free(SID);
}

void MediaServer_ProcessSUBSCRIBE(struct packetheader *header, struct ILibWebServer_Session *session)
{
   char* SID = NULL;
   int SIDLength = 0;
   char* Timeout = NULL;
   int TimeoutLength = 0;
   char* URL = NULL;
   int URLLength = 0;
   struct parser_result *p;
   
   struct packetheader_field_node *f;
   
   //
   // Iterate through all the HTTP Headers
   //
   f = header->FirstField;
   while(f!=NULL)
   {
      if(f->FieldLength==3 && strncasecmp(f->Field,"SID",3)==0)
      {
         //
         // Get the Subscription ID
         //
         SID = f->FieldData;
         SIDLength = f->FieldDataLength;
      }
      else if(f->FieldLength==8 && strncasecmp(f->Field,"Callback",8)==0)
      {
         //
         // Get the Callback URL
         //
         URL = f->FieldData;
         URLLength = f->FieldDataLength;
      }
      else if(f->FieldLength==7 && strncasecmp(f->Field,"Timeout",7)==0)
      {
         //
         // Get the requested timeout value
         //
         Timeout = f->FieldData;
         TimeoutLength = f->FieldDataLength;
      }
      
      f = f->NextField;
   }
   if(Timeout==NULL)
   {
      //
      // It a timeout wasn't specified, force it to a specific value
      //
      Timeout = "7200";
      TimeoutLength = 4;
   }
   else
   {
      p = ILibParseString(Timeout,0,TimeoutLength,"-",1);
      if(p->NumResults==2)
      {
         Timeout = p->LastResult->data;
         TimeoutLength = p->LastResult->datalength;
         if(TimeoutLength==8 && strncasecmp(Timeout,"INFINITE",8)==0)
         {
            //
            // Infinite timeouts will cause problems, so we don't allow it
            //
            Timeout = "7200";
            TimeoutLength = 4;
         }
      }
      else
      {
         Timeout = "7200";
         TimeoutLength = 4;
      }
      ILibDestructParserResults(p);
   }
   if(SID==NULL)
   {
      //
      // If not SID was specified, this is a subscription request
      //
      
      /* Subscribe */
      MediaServer_SubscribeEvents(header->DirectiveObj,header->DirectiveObjLength,Timeout,TimeoutLength,URL,URLLength,session);
   }
   else
   {
      //
      // If a SID was specified, it is a renewal request for an existing subscription
      //
      
      /* Renew */
      MediaServer_RenewEvents(header->DirectiveObj,header->DirectiveObjLength,SID,SIDLength,Timeout,TimeoutLength,session);
   }
}


void MediaServer_StreamDescriptionDocument_SCPD(struct ILibWebServer_Session *session, int StartActionList, char *buffer, int offset, int length, int DoneActionList, int Done)
{
   if(StartActionList)
   {
      ILibWebServer_StreamBody(session,"<?xml version=\"1.0\" encoding=\"utf-8\" ?><scpd xmlns=\"urn:schemas-upnp-org:service-1-0\"><specVersion><major>1</major><minor>0</minor></specVersion><actionList>",157,ILibAsyncSocket_MemoryOwnership_STATIC,0);
   }
   if(buffer!=NULL)
   {
      ILibWebServer_StreamBody(session,buffer+offset,length,ILibAsyncSocket_MemoryOwnership_USER,0);
   }
   if(DoneActionList)
   {
      ILibWebServer_StreamBody(session,"</actionList><serviceStateTable>",32,ILibAsyncSocket_MemoryOwnership_STATIC,0);
   }
   if(Done)
   {
      ILibWebServer_StreamBody(session,"</serviceStateTable></scpd>",27,ILibAsyncSocket_MemoryOwnership_STATIC,1);
   }
}

void MediaServer_ProcessHTTPPacket(struct ILibWebServer_Session *session, struct packetheader* header, char *bodyBuffer, int offset, int bodyBufferLength)
{
   
   
   int i;
   
   
   #if defined(WIN32) || defined(_WIN32_WCE)
   char *responseHeader = "\r\nCONTENT-TYPE:  text/xml; charset=\"utf-8\"\r\nServer: WINDOWS, UPnP/1.0, Intel MicroStack/1.0.2384";
   #else
   char *responseHeader = "\r\nCONTENT-TYPE:  text/xml; charset=\"utf-8\"\r\nServer: POSIX, UPnP/1.0, Intel MicroStack/1.0.2384";
   #endif
   char *errorTemplate = "HTTP/1.1 %d %s\r\nServer: %s, UPnP/1.0, Intel MicroStack/1.0.2384\r\nContent-Length: 0\r\n\r\n";
   char *errorPacket;
   int errorPacketLength;
   char *buffer;
   
   LVL3DEBUG(errorPacketLength=ILibGetRawPacket(header,&errorPacket);)
   LVL3DEBUG(printf("%s\r\n",errorPacket);)
   LVL3DEBUG(free(errorPacket);)			
   
   
   if(header->DirectiveLength==4 && memcmp(header->Directive,"HEAD",4)==0)
   {
      if(header->DirectiveObjLength==1 && memcmp(header->DirectiveObj,"/",1)==0)
      {
         //
         // A HEAD request for the device description document.
         // We stream the document back, so we don't return content length or anything
         // because the actual response won't have it either
         //
         ILibWebServer_StreamHeader_Raw(session,200,"OK",responseHeader,1);
         ILibWebServer_StreamBody(session,NULL,0,ILibAsyncSocket_MemoryOwnership_STATIC,1);
      }
      
      else if(header->DirectiveObjLength==27 && memcmp((header->DirectiveObj)+1,"ConnectionManager/scpd.xml",26)==0)
	{
		ILibWebServer_StreamHeader_Raw(session,200,"OK",responseHeader,1);
		ILibWebServer_StreamBody(session,NULL,0,ILibAsyncSocket_MemoryOwnership_STATIC,1);
	}
	else if(header->DirectiveObjLength==26 && memcmp((header->DirectiveObj)+1,"ContentDirectory/scpd.xml",25)==0)
	{
		ILibWebServer_StreamHeader_Raw(session,200,"OK",responseHeader,1);
		ILibWebServer_StreamBody(session,NULL,0,ILibAsyncSocket_MemoryOwnership_STATIC,1);
	}

      else
      {
         //
         // A HEAD request for something we don't have
         //
         errorPacket = (char*)malloc(128);
         errorPacketLength = sprintf(errorPacket,errorTemplate,404,"File Not Found",MediaServer_PLATFORM);
         ILibWebServer_Send_Raw(session,errorPacket,errorPacketLength,0,1);
      }
   }
   else if(header->DirectiveLength==3 && memcmp(header->Directive,"GET",3)==0)
   {
      if(header->DirectiveObjLength==1 && memcmp(header->DirectiveObj,"/",1)==0)
      {
         //
         // A GET Request for the device description document, so lets stream
         // it back to the client
         //
         
         MediaServer_StreamDescriptionDocument(session);
         
         
      }
      
      else if(header->DirectiveObjLength==27 && memcmp((header->DirectiveObj)+1,"ConnectionManager/scpd.xml",26)==0)
	{
		ILibWebServer_StreamHeader_Raw(session,200,"OK",responseHeader,1);
		MediaServer_StreamDescriptionDocument_SCPD(session,1,NULL,0,0,0,0);
		buffer = ILibDecompressString(MediaServer__ActionTable_ConnectionManager_Impl.Reserved,MediaServer__ActionTable_ConnectionManager_Impl.ReservedXL,MediaServer__ActionTable_ConnectionManager_Impl.ReservedUXL);
		if(MediaServer__Device_MediaServer_Impl.ConnectionManager->GetCurrentConnectionIDs!=NULL){MediaServer_StreamDescriptionDocument_SCPD(session,0,buffer,MediaServer__Device_MediaServer_Impl.ConnectionManager->GetCurrentConnectionIDs->Reserved,MediaServer__Device_MediaServer_Impl.ConnectionManager->GetCurrentConnectionIDs->Reserved2,0,0);}
		if(MediaServer__Device_MediaServer_Impl.ConnectionManager->GetCurrentConnectionInfo!=NULL){MediaServer_StreamDescriptionDocument_SCPD(session,0,buffer,MediaServer__Device_MediaServer_Impl.ConnectionManager->GetCurrentConnectionInfo->Reserved,MediaServer__Device_MediaServer_Impl.ConnectionManager->GetCurrentConnectionInfo->Reserved2,0,0);}
		if(MediaServer__Device_MediaServer_Impl.ConnectionManager->GetProtocolInfo!=NULL){MediaServer_StreamDescriptionDocument_SCPD(session,0,buffer,MediaServer__Device_MediaServer_Impl.ConnectionManager->GetProtocolInfo->Reserved,MediaServer__Device_MediaServer_Impl.ConnectionManager->GetProtocolInfo->Reserved2,0,0);}
		free(buffer);
		MediaServer_StreamDescriptionDocument_SCPD(session,0,NULL,0,0,1,0);
		buffer = ILibDecompressString(MediaServer__StateVariableTable_ConnectionManager_Impl.Reserved,MediaServer__StateVariableTable_ConnectionManager_Impl.ReservedXL,MediaServer__StateVariableTable_ConnectionManager_Impl.ReservedUXL);
		if(MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_ProtocolInfo!=NULL)
		{
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_ProtocolInfo->Reserved1,MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_ProtocolInfo->Reserved1L,ILibAsyncSocket_MemoryOwnership_USER,0);
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_ProtocolInfo->Reserved8,MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_ProtocolInfo->Reserved8L,ILibAsyncSocket_MemoryOwnership_USER,0);
		}
		if(MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_ConnectionStatus!=NULL)
		{
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_ConnectionStatus->Reserved1,MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_ConnectionStatus->Reserved1L,ILibAsyncSocket_MemoryOwnership_USER,0);
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_ConnectionStatus->Reserved2,MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_ConnectionStatus->Reserved2L,ILibAsyncSocket_MemoryOwnership_USER,0);
			for(i=0;i<MediaServer__StateVariable_AllowedValues_MAX;++i)
			{
				if(MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_ConnectionStatus->AllowedValues[i]!=NULL)
				{
					ILibWebServer_StreamBody(session,"<allowedValue>",14,ILibAsyncSocket_MemoryOwnership_STATIC,0);
					ILibWebServer_StreamBody(session,MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_ConnectionStatus->AllowedValues[i],(int)strlen(MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_ConnectionStatus->AllowedValues[i]),ILibAsyncSocket_MemoryOwnership_USER,0);
					ILibWebServer_StreamBody(session,"</allowedValue>",15,ILibAsyncSocket_MemoryOwnership_STATIC,0);
				}
			}
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_ConnectionStatus->Reserved3,MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_ConnectionStatus->Reserved3L,ILibAsyncSocket_MemoryOwnership_USER,0);
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_ConnectionStatus->Reserved8,MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_ConnectionStatus->Reserved8L,ILibAsyncSocket_MemoryOwnership_USER,0);
		}
		if(MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_AVTransportID!=NULL)
		{
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_AVTransportID->Reserved1,MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_AVTransportID->Reserved1L,ILibAsyncSocket_MemoryOwnership_USER,0);
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_AVTransportID->Reserved8,MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_AVTransportID->Reserved8L,ILibAsyncSocket_MemoryOwnership_USER,0);
		}
		if(MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_RcsID!=NULL)
		{
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_RcsID->Reserved1,MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_RcsID->Reserved1L,ILibAsyncSocket_MemoryOwnership_USER,0);
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_RcsID->Reserved8,MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_RcsID->Reserved8L,ILibAsyncSocket_MemoryOwnership_USER,0);
		}
		if(MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_ConnectionID!=NULL)
		{
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_ConnectionID->Reserved1,MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_ConnectionID->Reserved1L,ILibAsyncSocket_MemoryOwnership_USER,0);
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_ConnectionID->Reserved8,MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_ConnectionID->Reserved8L,ILibAsyncSocket_MemoryOwnership_USER,0);
		}
		if(MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_ConnectionManager!=NULL)
		{
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_ConnectionManager->Reserved1,MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_ConnectionManager->Reserved1L,ILibAsyncSocket_MemoryOwnership_USER,0);
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_ConnectionManager->Reserved8,MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_ConnectionManager->Reserved8L,ILibAsyncSocket_MemoryOwnership_USER,0);
		}
		if(MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_SourceProtocolInfo!=NULL)
		{
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_SourceProtocolInfo->Reserved1,MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_SourceProtocolInfo->Reserved1L,ILibAsyncSocket_MemoryOwnership_USER,0);
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_SourceProtocolInfo->Reserved8,MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_SourceProtocolInfo->Reserved8L,ILibAsyncSocket_MemoryOwnership_USER,0);
		}
		if(MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_SinkProtocolInfo!=NULL)
		{
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_SinkProtocolInfo->Reserved1,MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_SinkProtocolInfo->Reserved1L,ILibAsyncSocket_MemoryOwnership_USER,0);
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_SinkProtocolInfo->Reserved8,MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_SinkProtocolInfo->Reserved8L,ILibAsyncSocket_MemoryOwnership_USER,0);
		}
		if(MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_Direction!=NULL)
		{
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_Direction->Reserved1,MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_Direction->Reserved1L,ILibAsyncSocket_MemoryOwnership_USER,0);
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_Direction->Reserved2,MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_Direction->Reserved2L,ILibAsyncSocket_MemoryOwnership_USER,0);
			for(i=0;i<MediaServer__StateVariable_AllowedValues_MAX;++i)
			{
				if(MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_Direction->AllowedValues[i]!=NULL)
				{
					ILibWebServer_StreamBody(session,"<allowedValue>",14,ILibAsyncSocket_MemoryOwnership_STATIC,0);
					ILibWebServer_StreamBody(session,MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_Direction->AllowedValues[i],(int)strlen(MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_Direction->AllowedValues[i]),ILibAsyncSocket_MemoryOwnership_USER,0);
					ILibWebServer_StreamBody(session,"</allowedValue>",15,ILibAsyncSocket_MemoryOwnership_STATIC,0);
				}
			}
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_Direction->Reserved3,MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_Direction->Reserved3L,ILibAsyncSocket_MemoryOwnership_USER,0);
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_Direction->Reserved8,MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_A_ARG_TYPE_Direction->Reserved8L,ILibAsyncSocket_MemoryOwnership_USER,0);
		}
		if(MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_CurrentConnectionIDs!=NULL)
		{
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_CurrentConnectionIDs->Reserved1,MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_CurrentConnectionIDs->Reserved1L,ILibAsyncSocket_MemoryOwnership_USER,0);
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_CurrentConnectionIDs->Reserved8,MediaServer__Device_MediaServer_Impl.ConnectionManager->StateVar_CurrentConnectionIDs->Reserved8L,ILibAsyncSocket_MemoryOwnership_USER,0);
		}
		free(buffer);
		MediaServer_StreamDescriptionDocument_SCPD(session,0,NULL,0,0,0,1);
	}
	else if(header->DirectiveObjLength==26 && memcmp((header->DirectiveObj)+1,"ContentDirectory/scpd.xml",25)==0)
	{
		ILibWebServer_StreamHeader_Raw(session,200,"OK",responseHeader,1);
		MediaServer_StreamDescriptionDocument_SCPD(session,1,NULL,0,0,0,0);
		buffer = ILibDecompressString(MediaServer__ActionTable_ContentDirectory_Impl.Reserved,MediaServer__ActionTable_ContentDirectory_Impl.ReservedXL,MediaServer__ActionTable_ContentDirectory_Impl.ReservedUXL);
		if(MediaServer__Device_MediaServer_Impl.ContentDirectory->Browse!=NULL){MediaServer_StreamDescriptionDocument_SCPD(session,0,buffer,MediaServer__Device_MediaServer_Impl.ContentDirectory->Browse->Reserved,MediaServer__Device_MediaServer_Impl.ContentDirectory->Browse->Reserved2,0,0);}
		if(MediaServer__Device_MediaServer_Impl.ContentDirectory->CreateObject!=NULL){MediaServer_StreamDescriptionDocument_SCPD(session,0,buffer,MediaServer__Device_MediaServer_Impl.ContentDirectory->CreateObject->Reserved,MediaServer__Device_MediaServer_Impl.ContentDirectory->CreateObject->Reserved2,0,0);}
		if(MediaServer__Device_MediaServer_Impl.ContentDirectory->DestroyObject!=NULL){MediaServer_StreamDescriptionDocument_SCPD(session,0,buffer,MediaServer__Device_MediaServer_Impl.ContentDirectory->DestroyObject->Reserved,MediaServer__Device_MediaServer_Impl.ContentDirectory->DestroyObject->Reserved2,0,0);}
		if(MediaServer__Device_MediaServer_Impl.ContentDirectory->GetSearchCapabilities!=NULL){MediaServer_StreamDescriptionDocument_SCPD(session,0,buffer,MediaServer__Device_MediaServer_Impl.ContentDirectory->GetSearchCapabilities->Reserved,MediaServer__Device_MediaServer_Impl.ContentDirectory->GetSearchCapabilities->Reserved2,0,0);}
		if(MediaServer__Device_MediaServer_Impl.ContentDirectory->GetSortCapabilities!=NULL){MediaServer_StreamDescriptionDocument_SCPD(session,0,buffer,MediaServer__Device_MediaServer_Impl.ContentDirectory->GetSortCapabilities->Reserved,MediaServer__Device_MediaServer_Impl.ContentDirectory->GetSortCapabilities->Reserved2,0,0);}
		if(MediaServer__Device_MediaServer_Impl.ContentDirectory->GetSystemUpdateID!=NULL){MediaServer_StreamDescriptionDocument_SCPD(session,0,buffer,MediaServer__Device_MediaServer_Impl.ContentDirectory->GetSystemUpdateID->Reserved,MediaServer__Device_MediaServer_Impl.ContentDirectory->GetSystemUpdateID->Reserved2,0,0);}
		if(MediaServer__Device_MediaServer_Impl.ContentDirectory->Search!=NULL){MediaServer_StreamDescriptionDocument_SCPD(session,0,buffer,MediaServer__Device_MediaServer_Impl.ContentDirectory->Search->Reserved,MediaServer__Device_MediaServer_Impl.ContentDirectory->Search->Reserved2,0,0);}
		free(buffer);
		MediaServer_StreamDescriptionDocument_SCPD(session,0,NULL,0,0,1,0);
		buffer = ILibDecompressString(MediaServer__StateVariableTable_ContentDirectory_Impl.Reserved,MediaServer__StateVariableTable_ContentDirectory_Impl.ReservedXL,MediaServer__StateVariableTable_ContentDirectory_Impl.ReservedUXL);
		if(MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_BrowseFlag!=NULL)
		{
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_BrowseFlag->Reserved1,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_BrowseFlag->Reserved1L,ILibAsyncSocket_MemoryOwnership_USER,0);
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_BrowseFlag->Reserved2,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_BrowseFlag->Reserved2L,ILibAsyncSocket_MemoryOwnership_USER,0);
			for(i=0;i<MediaServer__StateVariable_AllowedValues_MAX;++i)
			{
				if(MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_BrowseFlag->AllowedValues[i]!=NULL)
				{
					ILibWebServer_StreamBody(session,"<allowedValue>",14,ILibAsyncSocket_MemoryOwnership_STATIC,0);
					ILibWebServer_StreamBody(session,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_BrowseFlag->AllowedValues[i],(int)strlen(MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_BrowseFlag->AllowedValues[i]),ILibAsyncSocket_MemoryOwnership_USER,0);
					ILibWebServer_StreamBody(session,"</allowedValue>",15,ILibAsyncSocket_MemoryOwnership_STATIC,0);
				}
			}
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_BrowseFlag->Reserved3,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_BrowseFlag->Reserved3L,ILibAsyncSocket_MemoryOwnership_USER,0);
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_BrowseFlag->Reserved8,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_BrowseFlag->Reserved8L,ILibAsyncSocket_MemoryOwnership_USER,0);
		}
		if(MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_ContainerUpdateIDs!=NULL)
		{
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_ContainerUpdateIDs->Reserved1,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_ContainerUpdateIDs->Reserved1L,ILibAsyncSocket_MemoryOwnership_USER,0);
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_ContainerUpdateIDs->Reserved8,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_ContainerUpdateIDs->Reserved8L,ILibAsyncSocket_MemoryOwnership_USER,0);
		}
		if(MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_SystemUpdateID!=NULL)
		{
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_SystemUpdateID->Reserved1,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_SystemUpdateID->Reserved1L,ILibAsyncSocket_MemoryOwnership_USER,0);
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_SystemUpdateID->Reserved8,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_SystemUpdateID->Reserved8L,ILibAsyncSocket_MemoryOwnership_USER,0);
		}
		if(MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_SortCriteria!=NULL)
		{
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_SortCriteria->Reserved1,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_SortCriteria->Reserved1L,ILibAsyncSocket_MemoryOwnership_USER,0);
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_SortCriteria->Reserved8,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_SortCriteria->Reserved8L,ILibAsyncSocket_MemoryOwnership_USER,0);
		}
		if(MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_SortCapabilities!=NULL)
		{
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_SortCapabilities->Reserved1,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_SortCapabilities->Reserved1L,ILibAsyncSocket_MemoryOwnership_USER,0);
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_SortCapabilities->Reserved8,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_SortCapabilities->Reserved8L,ILibAsyncSocket_MemoryOwnership_USER,0);
		}
		if(MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_UpdateID!=NULL)
		{
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_UpdateID->Reserved1,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_UpdateID->Reserved1L,ILibAsyncSocket_MemoryOwnership_USER,0);
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_UpdateID->Reserved8,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_UpdateID->Reserved8L,ILibAsyncSocket_MemoryOwnership_USER,0);
		}
		if(MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_Index!=NULL)
		{
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_Index->Reserved1,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_Index->Reserved1L,ILibAsyncSocket_MemoryOwnership_USER,0);
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_Index->Reserved8,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_Index->Reserved8L,ILibAsyncSocket_MemoryOwnership_USER,0);
		}
		if(MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_ObjectID!=NULL)
		{
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_ObjectID->Reserved1,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_ObjectID->Reserved1L,ILibAsyncSocket_MemoryOwnership_USER,0);
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_ObjectID->Reserved8,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_ObjectID->Reserved8L,ILibAsyncSocket_MemoryOwnership_USER,0);
		}
		if(MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_Count!=NULL)
		{
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_Count->Reserved1,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_Count->Reserved1L,ILibAsyncSocket_MemoryOwnership_USER,0);
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_Count->Reserved8,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_Count->Reserved8L,ILibAsyncSocket_MemoryOwnership_USER,0);
		}
		if(MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_Result!=NULL)
		{
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_Result->Reserved1,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_Result->Reserved1L,ILibAsyncSocket_MemoryOwnership_USER,0);
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_Result->Reserved8,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_Result->Reserved8L,ILibAsyncSocket_MemoryOwnership_USER,0);
		}
		if(MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_SearchCapabilities!=NULL)
		{
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_SearchCapabilities->Reserved1,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_SearchCapabilities->Reserved1L,ILibAsyncSocket_MemoryOwnership_USER,0);
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_SearchCapabilities->Reserved8,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_SearchCapabilities->Reserved8L,ILibAsyncSocket_MemoryOwnership_USER,0);
		}
		if(MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_SearchCriteria!=NULL)
		{
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_SearchCriteria->Reserved1,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_SearchCriteria->Reserved1L,ILibAsyncSocket_MemoryOwnership_USER,0);
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_SearchCriteria->Reserved8,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_SearchCriteria->Reserved8L,ILibAsyncSocket_MemoryOwnership_USER,0);
		}
		if(MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_Filter!=NULL)
		{
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_Filter->Reserved1,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_Filter->Reserved1L,ILibAsyncSocket_MemoryOwnership_USER,0);
			ILibWebServer_StreamBody(session,buffer+MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_Filter->Reserved8,MediaServer__Device_MediaServer_Impl.ContentDirectory->StateVar_A_ARG_TYPE_Filter->Reserved8L,ILibAsyncSocket_MemoryOwnership_USER,0);
		}
		free(buffer);
		MediaServer_StreamDescriptionDocument_SCPD(session,0,NULL,0,0,0,1);
	}

      else
      {
         //
         // A GET Request for something we don't have
         //
         errorPacket = (char*)malloc(128);
         errorPacketLength = sprintf(errorPacket,errorTemplate,404,"File Not Found",MediaServer_PLATFORM);
         ILibWebServer_Send_Raw(session,errorPacket,errorPacketLength,0,1);
      }
   }
   else if(header->DirectiveLength==4 && memcmp(header->Directive,"POST",4)==0)
   {
      //
      // Defer Control to the POST Handler
      //
      if(MediaServer_ProcessPOST(session,header,bodyBuffer,offset,bodyBufferLength)!=0)
      {
         //
         // A POST for an action that doesn't exist
         //
         MediaServer_Response_Error(session,401,"Invalid Action");
      }
   }
   
   else if(header->DirectiveLength==9 && memcmp(header->Directive,"SUBSCRIBE",9)==0)
   {
      //
      // Subscription Handler
      //
      MediaServer_ProcessSUBSCRIBE(header,session);
   }
   else if(header->DirectiveLength==11 && memcmp(header->Directive,"UNSUBSCRIBE",11)==0)
   {
      //
      // UnSubscribe Handler
      //
      MediaServer_ProcessUNSUBSCRIBE(header,session);
   }
   else
   {
      //
      // The client tried something we didn't expect/support
      //
      errorPacket = (char*)malloc(128);
      errorPacketLength = sprintf(errorPacket,errorTemplate,400,"Bad Request",MediaServer_PLATFORM);
      ILibWebServer_Send_Raw(session,errorPacket,errorPacketLength,ILibAsyncSocket_MemoryOwnership_CHAIN,1);
   }
}
void MediaServer_FragmentedSendNotify_Destroy(void *data);
void MediaServer_MasterPreSelect(void* object,fd_set *socketset, fd_set *writeset, fd_set *errorset, int* blocktime)
{
   int i;
   struct MediaServer_DataObject *MediaServer_Object = (struct MediaServer_DataObject*)object;	
   struct MediaServer_FragmentNotifyStruct *f;
   int timeout;
   
   if(MediaServer_Object->InitialNotify==0)
   {
      //
      // The initial "HELLO" packets were not sent yet, so lets send them
      //
      MediaServer_Object->InitialNotify = -1;
      //
      // In case we were interrupted, we need to flush out the caches of
      // all the control points by sending a "byebye" first, to insure
      // control points don't ignore our "hello" packets thinking they are just
      // periodic re-advertisements.
      //
      MediaServer_SendByeBye(MediaServer_Object);
      
      //
      // PacketNumber 0 is the controller, for the rest of the packets. Send
      // one of these to send out an advertisement "group"
      //
      f = (struct MediaServer_FragmentNotifyStruct*)malloc(sizeof(struct MediaServer_FragmentNotifyStruct));
      f->packetNumber=0;
      f->upnp = MediaServer_Object;
      //
      // We need to inject some delay in these packets to space them out,
      // otherwise we could overflow the inbound buffer of the recipient, causing them
      // to lose packets. And UPnP/1.0 control points are not as robust as UPnP/1.1 control points,
      // so they need all the help they can get ;)
      //
      timeout = (int)(0 + ((unsigned short)rand() % (500)));
      do
      {
         f->upnp->InitialNotify = rand();
      }while(f->upnp->InitialNotify==0);
      //
      // Register for the timed callback, to actually send the packet
      //
      ILibLifeTime_AddEx(f->upnp->WebServerTimer,f,timeout,&MediaServer_FragmentedSendNotify,&MediaServer_FragmentedSendNotify_Destroy);
      
   }
   if(MediaServer_Object->UpdateFlag!=0)
   {
      //
      // Somebody told us that we should recheck our IP Address table,
      // as one of them may have changed
      //
      MediaServer_Object->UpdateFlag = 0;
      
      /* Clear Sockets */
      
      
      //
      // Iterate through all the currently bound IP addresses
      // and release the sockets
      //
      for(i=0;i<MediaServer_Object->AddressListLength;++i)
      {
         ILibChain_SafeRemove(MediaServer_Object->Chain,MediaServer_Object->NOTIFY_SEND_socks[i]);
      }
      free(MediaServer_Object->NOTIFY_SEND_socks);
      
      for(i=0;i<MediaServer_Object->AddressListLength;++i)
      {
         ILibChain_SafeRemove(MediaServer_Object->Chain,MediaServer_Object->NOTIFY_RECEIVE_socks[i]);
      }
      free(MediaServer_Object->NOTIFY_RECEIVE_socks);
      
      
      //
      // Fetch a current list of ip addresses
      //
      free(MediaServer_Object->AddressList);
      MediaServer_Object->AddressListLength = ILibGetLocalIPAddressList(&(MediaServer_Object->AddressList));
      
      
      //
      // Re-Initialize our SEND socket
      //
      MediaServer_Object->NOTIFY_SEND_socks = (void**)malloc(sizeof(void*)*(MediaServer_Object->AddressListLength));
      MediaServer_Object->NOTIFY_RECEIVE_socks = (void**)malloc(sizeof(void*)*(MediaServer_Object->AddressListLength));
      
      //
      // Now that we have a new list of IP addresses, re-initialise everything
      //
      for(i=0;i<MediaServer_Object->AddressListLength;++i)
      {
         MediaServer_Object->NOTIFY_SEND_socks[i] = ILibAsyncUDPSocket_Create(
         MediaServer_Object->Chain,
         UPNP_MAX_SSDP_HEADER_SIZE,
         MediaServer_Object->AddressList[i],
         UPNP_PORT,
         ILibAsyncUDPSocket_Reuse_SHARED,
         NULL,
         NULL,
         MediaServer_Object);
         ILibAsyncUDPSocket_JoinMulticastGroup(
         MediaServer_Object->NOTIFY_SEND_socks[i],
         MediaServer_Object->AddressList[i],
         inet_addr(UPNP_GROUP));
         
         ILibAsyncUDPSocket_SetMulticastTTL(MediaServer_Object->NOTIFY_SEND_socks[i],UPNP_SSDP_TTL);
         
         MediaServer_Object->NOTIFY_RECEIVE_socks[i] = ILibAsyncUDPSocket_Create(
         MediaServer_Object->Chain,
         UPNP_MAX_SSDP_HEADER_SIZE,
         0,
         UPNP_PORT,
         ILibAsyncUDPSocket_Reuse_SHARED,
         &MediaServer_SSDPSink,
         NULL,
         MediaServer_Object);
         
         ILibAsyncUDPSocket_JoinMulticastGroup(
         MediaServer_Object->NOTIFY_RECEIVE_socks[i],
         MediaServer_Object->AddressList[i],
         inet_addr(UPNP_GROUP));
         
         
      }
      
      
      //
      // Iterate through all the packet types, and re-broadcast
      //
      for(i=1;i<=5;++i)
      {
         f = (struct MediaServer_FragmentNotifyStruct*)malloc(sizeof(struct MediaServer_FragmentNotifyStruct));
         f->packetNumber=i;
         f->upnp = MediaServer_Object;
         //
         // Inject some random delay, to spread these packets out, to help prevent
         // the inbound buffer of the recipient from overflowing, causing dropped packets.
         //
         timeout = (int)(0 + ((unsigned short)rand() % (500)));
         ILibLifeTime_AddEx(f->upnp->WebServerTimer,f,timeout,&MediaServer_FragmentedSendNotify,&MediaServer_FragmentedSendNotify_Destroy);
      }
   }	
}

void MediaServer_FragmentedSendNotify_Destroy(void *data)
{
   free(data);
}
void MediaServer_FragmentedSendNotify(void *data)
{
   struct MediaServer_FragmentNotifyStruct *FNS = (struct MediaServer_FragmentNotifyStruct*)data;
   int timeout,timeout2;
   int subsetRange;
   int packetlength;
   char* packet = (char*)malloc(5000);
   int i,i2;
   struct MediaServer_FragmentNotifyStruct *f;
   
   if(FNS->packetNumber==0)
   {				
      subsetRange = 5000/5; // Make sure all our packets will get out within 5 seconds
      
      // Send the first "group"
      for(i2=0;i2<5;++i2)
      {
         f = (struct MediaServer_FragmentNotifyStruct*)malloc(sizeof(struct MediaServer_FragmentNotifyStruct));
         f->packetNumber=i2+1;
         f->upnp = FNS->upnp;
         timeout2 = (rand() % subsetRange);
         ILibLifeTime_AddEx(FNS->upnp->WebServerTimer,f,timeout2,&MediaServer_FragmentedSendNotify,&MediaServer_FragmentedSendNotify_Destroy);
      }
      
      // Now Repeat this "group" after 7 seconds, to insure there is no overlap
      for(i2=0;i2<5;++i2)
      {
         f = (struct MediaServer_FragmentNotifyStruct*)malloc(sizeof(struct MediaServer_FragmentNotifyStruct));
         f->packetNumber=i2+1;
         f->upnp = FNS->upnp;
         timeout2 = 7000 + (rand() % subsetRange);
         ILibLifeTime_AddEx(FNS->upnp->WebServerTimer,f,timeout2,&MediaServer_FragmentedSendNotify,&MediaServer_FragmentedSendNotify_Destroy);
      }
      
      // Calculate the next transmission window and spread the packets
      timeout = (int)((FNS->upnp->NotifyCycleTime/4) + ((unsigned short)rand() % (FNS->upnp->NotifyCycleTime/2 - FNS->upnp->NotifyCycleTime/4)));
      ILibLifeTime_Add(FNS->upnp->WebServerTimer,FNS,timeout,&MediaServer_FragmentedSendNotify,&MediaServer_FragmentedSendNotify_Destroy);
   }
   
   for(i=0;i<FNS->upnp->AddressListLength;++i)
   {
      ILibAsyncUDPSocket_SetMulticastInterface(FNS->upnp->NOTIFY_SEND_socks[i],FNS->upnp->AddressList[i]);
      switch(FNS->packetNumber)
      {
         case 1:
						MediaServer_BuildSsdpNotifyPacket(packet,&packetlength,FNS->upnp->AddressList[i],(unsigned short)FNS->upnp->WebSocketPortNumber,0,FNS->upnp->UDN,"::upnp:rootdevice","upnp:rootdevice","",FNS->upnp->NotifyCycleTime);
						ILibAsyncUDPSocket_SendTo(FNS->upnp->NOTIFY_SEND_socks[i],inet_addr(UPNP_GROUP),UPNP_PORT,packet,packetlength,ILibAsyncSocket_MemoryOwnership_USER);
						break;
					case 2:
						MediaServer_BuildSsdpNotifyPacket(packet,&packetlength,FNS->upnp->AddressList[i],(unsigned short)FNS->upnp->WebSocketPortNumber,0,FNS->upnp->UDN,"","uuid:",FNS->upnp->UDN,FNS->upnp->NotifyCycleTime);
						ILibAsyncUDPSocket_SendTo(FNS->upnp->NOTIFY_SEND_socks[i],inet_addr(UPNP_GROUP),UPNP_PORT,packet,packetlength,ILibAsyncSocket_MemoryOwnership_USER);
						break;
					case 3:
						MediaServer_BuildSsdpNotifyPacket(packet,&packetlength,FNS->upnp->AddressList[i],(unsigned short)FNS->upnp->WebSocketPortNumber,0,FNS->upnp->UDN,"::urn:schemas-upnp-org:device:MediaServer:1","urn:schemas-upnp-org:device:MediaServer:1","",FNS->upnp->NotifyCycleTime);
						ILibAsyncUDPSocket_SendTo(FNS->upnp->NOTIFY_SEND_socks[i],inet_addr(UPNP_GROUP),UPNP_PORT,packet,packetlength,ILibAsyncSocket_MemoryOwnership_USER);
						break;
					case 4:
						MediaServer_BuildSsdpNotifyPacket(packet,&packetlength,FNS->upnp->AddressList[i],(unsigned short)FNS->upnp->WebSocketPortNumber,0,FNS->upnp->UDN,"::urn:schemas-upnp-org:service:ConnectionManager:1","urn:schemas-upnp-org:service:ConnectionManager:1","",FNS->upnp->NotifyCycleTime);
						ILibAsyncUDPSocket_SendTo(FNS->upnp->NOTIFY_SEND_socks[i],inet_addr(UPNP_GROUP),UPNP_PORT,packet,packetlength,ILibAsyncSocket_MemoryOwnership_USER);
						break;
					case 5:
						MediaServer_BuildSsdpNotifyPacket(packet,&packetlength,FNS->upnp->AddressList[i],(unsigned short)FNS->upnp->WebSocketPortNumber,0,FNS->upnp->UDN,"::urn:schemas-upnp-org:service:ContentDirectory:1","urn:schemas-upnp-org:service:ContentDirectory:1","",FNS->upnp->NotifyCycleTime);
						ILibAsyncUDPSocket_SendTo(FNS->upnp->NOTIFY_SEND_socks[i],inet_addr(UPNP_GROUP),UPNP_PORT,packet,packetlength,ILibAsyncSocket_MemoryOwnership_USER);
						break;
					
         
      }
   }
   free(packet);
   if(FNS->packetNumber!=0)
   {
      free(FNS);
   }
}
void MediaServer_SendNotify(const struct MediaServer_DataObject *upnp)
{
   int packetlength;
   char* packet = (char*)malloc(5000);
   int i,i2;
   
   for(i=0;i<upnp->AddressListLength;++i)
   {
      ILibAsyncUDPSocket_SetMulticastInterface(upnp->NOTIFY_SEND_socks[i],upnp->AddressList[i]);
      for (i2=0;i2<2;i2++)
      {
         MediaServer_BuildSsdpNotifyPacket(packet,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::upnp:rootdevice","upnp:rootdevice","",upnp->NotifyCycleTime);
					ILibAsyncUDPSocket_SendTo(upnp->NOTIFY_SEND_socks[i],inet_addr(UPNP_GROUP),UPNP_PORT,packet,packetlength,ILibAsyncSocket_MemoryOwnership_USER);
			MediaServer_BuildSsdpNotifyPacket(packet,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"","uuid:",upnp->UDN,upnp->NotifyCycleTime);
			ILibAsyncUDPSocket_SendTo(upnp->NOTIFY_SEND_socks[i],inet_addr(UPNP_GROUP),UPNP_PORT,packet,packetlength,ILibAsyncSocket_MemoryOwnership_USER);
			MediaServer_BuildSsdpNotifyPacket(packet,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:device:MediaServer:1","urn:schemas-upnp-org:device:MediaServer:1","",upnp->NotifyCycleTime);
			ILibAsyncUDPSocket_SendTo(upnp->NOTIFY_SEND_socks[i],inet_addr(UPNP_GROUP),UPNP_PORT,packet,packetlength,ILibAsyncSocket_MemoryOwnership_USER);
			MediaServer_BuildSsdpNotifyPacket(packet,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:ConnectionManager:1","urn:schemas-upnp-org:service:ConnectionManager:1","",upnp->NotifyCycleTime);
			ILibAsyncUDPSocket_SendTo(upnp->NOTIFY_SEND_socks[i],inet_addr(UPNP_GROUP),UPNP_PORT,packet,packetlength,ILibAsyncSocket_MemoryOwnership_USER);
			MediaServer_BuildSsdpNotifyPacket(packet,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:ContentDirectory:1","urn:schemas-upnp-org:service:ContentDirectory:1","",upnp->NotifyCycleTime);
			ILibAsyncUDPSocket_SendTo(upnp->NOTIFY_SEND_socks[i],inet_addr(UPNP_GROUP),UPNP_PORT,packet,packetlength,ILibAsyncSocket_MemoryOwnership_USER);

      }
   }
   free(packet);
}

#define MediaServer_BuildSsdpByeByePacket(outpacket,outlength,USN,USNex,NT,NTex,DeviceID)\
{\
   if(DeviceID==0)\
   {\
      *outlength = sprintf(outpacket,"NOTIFY * HTTP/1.1\r\nHOST: 239.255.255.250:1900\r\nNTS: ssdp:byebye\r\nUSN: uuid:%s%s\r\nNT: %s%s\r\nContent-Length: 0\r\n\r\n",USN,USNex,NT,NTex);\
   }\
   else\
   {\
      if(memcmp(NT,"uuid:",5)==0)\
      {\
         *outlength = sprintf(outpacket,"NOTIFY * HTTP/1.1\r\nHOST: 239.255.255.250:1900\r\nNTS: ssdp:byebye\r\nUSN: uuid:%s_%d%s\r\nNT: %s%s_%d\r\nContent-Length: 0\r\n\r\n",USN,DeviceID,USNex,NT,NTex,DeviceID);\
      }\
      else\
      {\
         *outlength = sprintf(outpacket,"NOTIFY * HTTP/1.1\r\nHOST: 239.255.255.250:1900\r\nNTS: ssdp:byebye\r\nUSN: uuid:%s_%d%s\r\nNT: %s%s\r\nContent-Length: 0\r\n\r\n",USN,DeviceID,USNex,NT,NTex);\
      }\
   }\
}


void MediaServer_SendByeBye(const struct MediaServer_DataObject *upnp)
{
   
   int packetlength;
   char* packet = (char*)malloc(5000);
   int i, i2;
   
   for(i=0;i<upnp->AddressListLength;++i)
   {	
      ILibAsyncUDPSocket_SetMulticastInterface(upnp->NOTIFY_SEND_socks[i],upnp->AddressList[i]);
      
      for (i2=0;i2<2;i2++)
      {
         MediaServer_BuildSsdpByeByePacket(packet,&packetlength,upnp->UDN,"::upnp:rootdevice","upnp:rootdevice","",0);
					ILibAsyncUDPSocket_SendTo(upnp->NOTIFY_SEND_socks[i],inet_addr(UPNP_GROUP),UPNP_PORT,packet,packetlength,ILibAsyncSocket_MemoryOwnership_USER);
      MediaServer_BuildSsdpByeByePacket(packet,&packetlength,upnp->UDN,"","uuid:",upnp->UDN,0);
      ILibAsyncUDPSocket_SendTo(upnp->NOTIFY_SEND_socks[i],inet_addr(UPNP_GROUP),UPNP_PORT,packet,packetlength,ILibAsyncSocket_MemoryOwnership_USER);
      MediaServer_BuildSsdpByeByePacket(packet,&packetlength,upnp->UDN,"::urn:schemas-upnp-org:device:MediaServer:1","urn:schemas-upnp-org:device:MediaServer:1","",0);
      ILibAsyncUDPSocket_SendTo(upnp->NOTIFY_SEND_socks[i],inet_addr(UPNP_GROUP),UPNP_PORT,packet,packetlength,ILibAsyncSocket_MemoryOwnership_USER);
      MediaServer_BuildSsdpByeByePacket(packet,&packetlength,upnp->UDN,"::urn:schemas-upnp-org:service:ConnectionManager:1","urn:schemas-upnp-org:service:ConnectionManager:1","",0);
      ILibAsyncUDPSocket_SendTo(upnp->NOTIFY_SEND_socks[i],inet_addr(UPNP_GROUP),UPNP_PORT,packet,packetlength,ILibAsyncSocket_MemoryOwnership_USER);
      MediaServer_BuildSsdpByeByePacket(packet,&packetlength,upnp->UDN,"::urn:schemas-upnp-org:service:ContentDirectory:1","urn:schemas-upnp-org:service:ContentDirectory:1","",0);
      ILibAsyncUDPSocket_SendTo(upnp->NOTIFY_SEND_socks[i],inet_addr(UPNP_GROUP),UPNP_PORT,packet,packetlength,ILibAsyncSocket_MemoryOwnership_USER);

      }
   }
   free(packet);
}

/*! \fn MediaServer_Response_Error(const MediaServer_SessionToken MediaServer_Token, const int ErrorCode, const char* ErrorMsg)
\brief Responds to the client invocation with a SOAP Fault
\param MediaServer_Token UPnP token
\param ErrorCode Fault Code
\param ErrorMsg Error Detail
*/
void MediaServer_Response_Error(const MediaServer_SessionToken MediaServer_Token, const int ErrorCode, const char* ErrorMsg)
{
   char* body;
   int bodylength;
   char* head;
   int headlength;
   body = (char*)malloc(395 + (int)strlen(ErrorMsg));
   bodylength = sprintf(body,"<s:Envelope\r\n xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><s:Fault><faultcode>s:Client</faultcode><faultstring>UPnPError</faultstring><detail><UPnPError xmlns=\"urn:schemas-upnp-org:control-1-0\"><errorCode>%d</errorCode><errorDescription>%s</errorDescription></UPnPError></detail></s:Fault></s:Body></s:Envelope>",ErrorCode,ErrorMsg);
   head = (char*)malloc(59);
   headlength = sprintf(head,"HTTP/1.1 500 Internal\r\nContent-Length: %d\r\n\r\n",bodylength);
   ILibWebServer_Send_Raw((struct ILibWebServer_Session*)MediaServer_Token,head,headlength,0,0);
   ILibWebServer_Send_Raw((struct ILibWebServer_Session*)MediaServer_Token,body,bodylength,0,1);
}

/*! \fn MediaServer_GetLocalInterfaceToHost(const MediaServer_SessionToken MediaServer_Token)
\brief When a UPnP request is dispatched, this method determines which ip address actually received this request
\param MediaServer_Token UPnP token
\returns IP Address
*/
int MediaServer_GetLocalInterfaceToHost(const MediaServer_SessionToken MediaServer_Token)
{
   return(ILibWebServer_GetLocalInterface((struct ILibWebServer_Session*)MediaServer_Token));
}

void MediaServer_ResponseGeneric(const MediaServer_MicroStackToken MediaServer_Token,const char* ServiceURI,const char* MethodName,const char* Params)
{
   char* packet;
   int packetlength;
   struct ILibWebServer_Session *session = (struct ILibWebServer_Session*)MediaServer_Token;
   int RVAL=0;
   
   packet = (char*)malloc(239+strlen(ServiceURI)+strlen(Params)+(strlen(MethodName)*2));
   packetlength = sprintf(packet,"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"><s:Body><u:%sResponse xmlns:u=\"%s\">%s</u:%sResponse></s:Body></s:Envelope>",MethodName,ServiceURI,Params,MethodName);
   LVL3DEBUG(printf("SendBody: %s\r\n",packet);)
   #if defined(WIN32) || defined(_WIN32_WCE)
   RVAL=ILibWebServer_StreamHeader_Raw(session,200,"OK","\r\nEXT:\r\nCONTENT-TYPE: text/xml; charset=\"utf-8\"\r\nSERVER: WINDOWS, UPnP/1.0, Intel MicroStack/1.0.2384",1);
   #else
   RVAL=ILibWebServer_StreamHeader_Raw(session,200,"OK","\r\nEXT:\r\nCONTENT-TYPE: text/xml; charset=\"utf-8\"\r\nSERVER: POSIX, UPnP/1.0, Intel MicroStack/1.0.2384",1);
   #endif
   if(RVAL!=ILibAsyncSocket_SEND_ON_CLOSED_SOCKET_ERROR && RVAL != ILibWebServer_SEND_RESULTED_IN_DISCONNECT)
   {
      RVAL=ILibWebServer_StreamBody(session,packet,packetlength,0,1);
   }
}

/*! \fn MediaServer_Response_ConnectionManager_GetCurrentConnectionIDs(const MediaServer_SessionToken MediaServer_Token, const char* unescaped_ConnectionIDs)
	\brief Response Method for ConnectionManager >> urn:schemas-upnp-org:service:ConnectionManager:1 >> GetCurrentConnectionIDs
	\param MediaServer_Token MicroStack token
 \param unescaped_ConnectionIDs Value of argument ConnectionIDs \b     Note: Automatically Escaped
*/
void MediaServer_Response_ConnectionManager_GetCurrentConnectionIDs(const MediaServer_SessionToken MediaServer_Token, const char* unescaped_ConnectionIDs)
{
  char* body;
	char *ConnectionIDs = (char*)malloc(1+ILibXmlEscapeLength(unescaped_ConnectionIDs));

	ILibXmlEscape(ConnectionIDs,unescaped_ConnectionIDs);
  body = (char*)malloc(32+strlen(ConnectionIDs));
  sprintf(body,"<ConnectionIDs>%s</ConnectionIDs>",ConnectionIDs);
  MediaServer_ResponseGeneric(MediaServer_Token,"urn:schemas-upnp-org:service:ConnectionManager:1","GetCurrentConnectionIDs",body);
  free(body);
	free(ConnectionIDs);
}

/*! \fn MediaServer_Response_ConnectionManager_GetCurrentConnectionInfo(const MediaServer_SessionToken MediaServer_Token, const int RcsID, const int AVTransportID, const char* unescaped_ProtocolInfo, const char* unescaped_PeerConnectionManager, const int PeerConnectionID, const char* unescaped_Direction, const char* unescaped_Status)
	\brief Response Method for ConnectionManager >> urn:schemas-upnp-org:service:ConnectionManager:1 >> GetCurrentConnectionInfo
	\param MediaServer_Token MicroStack token
 \param RcsID Value of argument RcsID
 \param AVTransportID Value of argument AVTransportID
 \param unescaped_ProtocolInfo Value of argument ProtocolInfo \b     Note: Automatically Escaped
 \param unescaped_PeerConnectionManager Value of argument PeerConnectionManager \b     Note: Automatically Escaped
 \param PeerConnectionID Value of argument PeerConnectionID
 \param unescaped_Direction Value of argument Direction \b     Note: Automatically Escaped
 \param unescaped_Status Value of argument Status \b     Note: Automatically Escaped
*/
void MediaServer_Response_ConnectionManager_GetCurrentConnectionInfo(const MediaServer_SessionToken MediaServer_Token, const int RcsID, const int AVTransportID, const char* unescaped_ProtocolInfo, const char* unescaped_PeerConnectionManager, const int PeerConnectionID, const char* unescaped_Direction, const char* unescaped_Status)
{
  char* body;
	char *ProtocolInfo = (char*)malloc(1+ILibXmlEscapeLength(unescaped_ProtocolInfo));
	char *PeerConnectionManager = (char*)malloc(1+ILibXmlEscapeLength(unescaped_PeerConnectionManager));
	char *Direction = (char*)malloc(1+ILibXmlEscapeLength(unescaped_Direction));
	char *Status = (char*)malloc(1+ILibXmlEscapeLength(unescaped_Status));

	ILibXmlEscape(ProtocolInfo,unescaped_ProtocolInfo);
	ILibXmlEscape(PeerConnectionManager,unescaped_PeerConnectionManager);
	ILibXmlEscape(Direction,unescaped_Direction);
	ILibXmlEscape(Status,unescaped_Status);
  body = (char*)malloc(233+strlen(ProtocolInfo)+strlen(PeerConnectionManager)+strlen(Direction)+strlen(Status));
  sprintf(body,"<RcsID>%d</RcsID><AVTransportID>%d</AVTransportID><ProtocolInfo>%s</ProtocolInfo><PeerConnectionManager>%s</PeerConnectionManager><PeerConnectionID>%d</PeerConnectionID><Direction>%s</Direction><Status>%s</Status>",RcsID,AVTransportID,ProtocolInfo,PeerConnectionManager,PeerConnectionID,Direction,Status);
  MediaServer_ResponseGeneric(MediaServer_Token,"urn:schemas-upnp-org:service:ConnectionManager:1","GetCurrentConnectionInfo",body);
  free(body);
	free(ProtocolInfo);
	free(PeerConnectionManager);
	free(Direction);
	free(Status);
}

/*! \fn MediaServer_Response_ConnectionManager_GetProtocolInfo(const MediaServer_SessionToken MediaServer_Token, const char* unescaped_Source, const char* unescaped_Sink)
	\brief Response Method for ConnectionManager >> urn:schemas-upnp-org:service:ConnectionManager:1 >> GetProtocolInfo
	\param MediaServer_Token MicroStack token
 \param unescaped_Source Value of argument Source \b     Note: Automatically Escaped
 \param unescaped_Sink Value of argument Sink \b     Note: Automatically Escaped
*/
void MediaServer_Response_ConnectionManager_GetProtocolInfo(const MediaServer_SessionToken MediaServer_Token, const char* unescaped_Source, const char* unescaped_Sink)
{
  char* body;
	char *Source = (char*)malloc(1+ILibXmlEscapeLength(unescaped_Source));
	char *Sink = (char*)malloc(1+ILibXmlEscapeLength(unescaped_Sink));

	ILibXmlEscape(Source,unescaped_Source);
	ILibXmlEscape(Sink,unescaped_Sink);
  body = (char*)malloc(31+strlen(Source)+strlen(Sink));
  sprintf(body,"<Source>%s</Source><Sink>%s</Sink>",Source,Sink);
  MediaServer_ResponseGeneric(MediaServer_Token,"urn:schemas-upnp-org:service:ConnectionManager:1","GetProtocolInfo",body);
  free(body);
	free(Source);
	free(Sink);
}

/*! \fn MediaServer_Response_ContentDirectory_Browse(const MediaServer_SessionToken MediaServer_Token, const char* Result, const unsigned int NumberReturned, const unsigned int TotalMatches, const unsigned int UpdateID)
	\brief Response Method for ContentDirectory >> urn:schemas-upnp-org:service:ContentDirectory:1 >> Browse
	\param MediaServer_Token MicroStack token
 \param Result Value of argument Result \b     Note: Must be escaped
 \param NumberReturned Value of argument NumberReturned
 \param TotalMatches Value of argument TotalMatches
 \param UpdateID Value of argument UpdateID
*/
void MediaServer_Response_ContentDirectory_Browse(const MediaServer_SessionToken MediaServer_Token, const char* Result, const unsigned int NumberReturned, const unsigned int TotalMatches, const unsigned int UpdateID)
{
  char* body;

  body = (char*)malloc(134+strlen(Result));
  sprintf(body,"<Result>%s</Result><NumberReturned>%u</NumberReturned><TotalMatches>%u</TotalMatches><UpdateID>%u</UpdateID>",Result,NumberReturned,TotalMatches,UpdateID);
  MediaServer_ResponseGeneric(MediaServer_Token,"urn:schemas-upnp-org:service:ContentDirectory:1","Browse",body);
  free(body);
}

/*! \fn MediaServer_Response_ContentDirectory_CreateObject(const MediaServer_SessionToken MediaServer_Token, const char* ObjectID, const char* Result)
	\brief Response Method for ContentDirectory >> urn:schemas-upnp-org:service:ContentDirectory:1 >> CreateObject
	\param MediaServer_Token MicroStack token
 \param ObjectID Value of argument ObjectID \b     Note: Must be escaped
 \param Result Value of argument Result \b     Note: Must be escaped
*/
void MediaServer_Response_ContentDirectory_CreateObject(const MediaServer_SessionToken MediaServer_Token, const char* ObjectID, const char* Result)
{
  char* body;

  body = (char*)malloc(39+strlen(ObjectID)+strlen(Result));
  sprintf(body,"<ObjectID>%s</ObjectID><Result>%s</Result>",ObjectID,Result);
  MediaServer_ResponseGeneric(MediaServer_Token,"urn:schemas-upnp-org:service:ContentDirectory:1","CreateObject",body);
  free(body);
}

/*! \fn MediaServer_Response_ContentDirectory_DestroyObject(const MediaServer_SessionToken MediaServer_Token)
	\brief Response Method for ContentDirectory >> urn:schemas-upnp-org:service:ContentDirectory:1 >> DestroyObject
	\param MediaServer_Token MicroStack token
*/
void MediaServer_Response_ContentDirectory_DestroyObject(const MediaServer_SessionToken MediaServer_Token)
{
MediaServer_ResponseGeneric(MediaServer_Token,"urn:schemas-upnp-org:service:ContentDirectory:1","DestroyObject","");
}

/*! \fn MediaServer_Response_ContentDirectory_GetSearchCapabilities(const MediaServer_SessionToken MediaServer_Token, const char* unescaped_SearchCaps)
	\brief Response Method for ContentDirectory >> urn:schemas-upnp-org:service:ContentDirectory:1 >> GetSearchCapabilities
	\param MediaServer_Token MicroStack token
 \param unescaped_SearchCaps Value of argument SearchCaps \b     Note: Automatically Escaped
*/
void MediaServer_Response_ContentDirectory_GetSearchCapabilities(const MediaServer_SessionToken MediaServer_Token, const char* unescaped_SearchCaps)
{
  char* body;
	char *SearchCaps = (char*)malloc(1+ILibXmlEscapeLength(unescaped_SearchCaps));

	ILibXmlEscape(SearchCaps,unescaped_SearchCaps);
  body = (char*)malloc(26+strlen(SearchCaps));
  sprintf(body,"<SearchCaps>%s</SearchCaps>",SearchCaps);
  MediaServer_ResponseGeneric(MediaServer_Token,"urn:schemas-upnp-org:service:ContentDirectory:1","GetSearchCapabilities",body);
  free(body);
	free(SearchCaps);
}

/*! \fn MediaServer_Response_ContentDirectory_GetSortCapabilities(const MediaServer_SessionToken MediaServer_Token, const char* unescaped_SortCaps)
	\brief Response Method for ContentDirectory >> urn:schemas-upnp-org:service:ContentDirectory:1 >> GetSortCapabilities
	\param MediaServer_Token MicroStack token
 \param unescaped_SortCaps Value of argument SortCaps \b     Note: Automatically Escaped
*/
void MediaServer_Response_ContentDirectory_GetSortCapabilities(const MediaServer_SessionToken MediaServer_Token, const char* unescaped_SortCaps)
{
  char* body;
	char *SortCaps = (char*)malloc(1+ILibXmlEscapeLength(unescaped_SortCaps));

	ILibXmlEscape(SortCaps,unescaped_SortCaps);
  body = (char*)malloc(22+strlen(SortCaps));
  sprintf(body,"<SortCaps>%s</SortCaps>",SortCaps);
  MediaServer_ResponseGeneric(MediaServer_Token,"urn:schemas-upnp-org:service:ContentDirectory:1","GetSortCapabilities",body);
  free(body);
	free(SortCaps);
}

/*! \fn MediaServer_Response_ContentDirectory_GetSystemUpdateID(const MediaServer_SessionToken MediaServer_Token, const unsigned int Id)
	\brief Response Method for ContentDirectory >> urn:schemas-upnp-org:service:ContentDirectory:1 >> GetSystemUpdateID
	\param MediaServer_Token MicroStack token
 \param Id Value of argument Id
*/
void MediaServer_Response_ContentDirectory_GetSystemUpdateID(const MediaServer_SessionToken MediaServer_Token, const unsigned int Id)
{
  char* body;

  body = (char*)malloc(21);
  sprintf(body,"<Id>%u</Id>",Id);
  MediaServer_ResponseGeneric(MediaServer_Token,"urn:schemas-upnp-org:service:ContentDirectory:1","GetSystemUpdateID",body);
  free(body);
}

/*! \fn MediaServer_Response_ContentDirectory_Search(const MediaServer_SessionToken MediaServer_Token, const char* Result, const unsigned int NumberReturned, const unsigned int TotalMatches, const unsigned int UpdateID)
	\brief Response Method for ContentDirectory >> urn:schemas-upnp-org:service:ContentDirectory:1 >> Search
	\param MediaServer_Token MicroStack token
 \param Result Value of argument Result \b     Note: Must be escaped
 \param NumberReturned Value of argument NumberReturned
 \param TotalMatches Value of argument TotalMatches
 \param UpdateID Value of argument UpdateID
*/
void MediaServer_Response_ContentDirectory_Search(const MediaServer_SessionToken MediaServer_Token, const char* Result, const unsigned int NumberReturned, const unsigned int TotalMatches, const unsigned int UpdateID)
{
  char* body;

  body = (char*)malloc(134+strlen(Result));
  sprintf(body,"<Result>%s</Result><NumberReturned>%u</NumberReturned><TotalMatches>%u</TotalMatches><UpdateID>%u</UpdateID>",Result,NumberReturned,TotalMatches,UpdateID);
  MediaServer_ResponseGeneric(MediaServer_Token,"urn:schemas-upnp-org:service:ContentDirectory:1","Search",body);
  free(body);
}



void MediaServer_SendEventSink(
void *WebReaderToken,
int IsInterrupt,
struct packetheader *header,
char *buffer,
int *p_BeginPointer,
int EndPointer,
int done,
void *subscriber,
void *upnp,
int *PAUSE)	
{
   if(done!=0 && ((struct SubscriberInfo*)subscriber)->Disposing==0)
   {
      sem_wait(&(((struct MediaServer_DataObject*)upnp)->EventLock));
      --((struct SubscriberInfo*)subscriber)->RefCount;
      if(((struct SubscriberInfo*)subscriber)->RefCount==0)
      {
         LVL3DEBUG(printf("\r\n\r\nSubscriber at [%s] %d.%d.%d.%d:%d was/did UNSUBSCRIBE while trying to send event\r\n\r\n",((struct SubscriberInfo*)subscriber)->SID,(((struct SubscriberInfo*)subscriber)->Address&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>8)&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>16)&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>24)&0xFF),((struct SubscriberInfo*)subscriber)->Port);)
         MediaServer_DestructSubscriberInfo(((struct SubscriberInfo*)subscriber));
      }
      else if(header==NULL)
      {
         LVL3DEBUG(printf("\r\n\r\nCould not deliver event for [%s] %d.%d.%d.%d:%d UNSUBSCRIBING\r\n\r\n",((struct SubscriberInfo*)subscriber)->SID,(((struct SubscriberInfo*)subscriber)->Address&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>8)&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>16)&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>24)&0xFF),((struct SubscriberInfo*)subscriber)->Port);)
         // Could not send Event, so unsubscribe the subscriber
         ((struct SubscriberInfo*)subscriber)->Disposing = 1;
         MediaServer_ExpireSubscriberInfo(upnp,subscriber);
      }
      sem_post(&(((struct MediaServer_DataObject*)upnp)->EventLock));
   }
}
void MediaServer_SendEvent_Body(void *upnptoken,char *body,int bodylength,struct SubscriberInfo *info)
{
   struct MediaServer_DataObject* MediaServer_Object = (struct MediaServer_DataObject*)upnptoken;
   struct sockaddr_in dest;
   int packetLength;
   char *packet;
   int ipaddr;
   
   memset(&dest,0,sizeof(dest));
   dest.sin_addr.s_addr = info->Address;
   dest.sin_port = htons(info->Port);
   dest.sin_family = AF_INET;
   ipaddr = info->Address;
   
   packet = (char*)malloc(info->PathLength + bodylength + 483);
   packetLength = sprintf(packet,"NOTIFY %s HTTP/1.1\r\nSERVER: %s, UPnP/1.0, Intel MicroStack/1.0.2384\r\nHOST: %s:%d\r\nContent-Type: text/xml; charset=\"utf-8\"\r\nNT: upnp:event\r\nNTS: upnp:propchange\r\nSID: %s\r\nSEQ: %d\r\nContent-Length: %d\r\n\r\n<?xml version=\"1.0\" encoding=\"utf-8\"?><e:propertyset xmlns:e=\"urn:schemas-upnp-org:event-1-0\"><e:property><%s></e:property></e:propertyset>",info->Path,MediaServer_PLATFORM,inet_ntoa(dest.sin_addr),info->Port,info->SID,info->SEQ,bodylength+137,body);
   ++info->SEQ;
   
   ++info->RefCount;
   ILibWebClient_PipelineRequestEx(MediaServer_Object->EventClient,&dest,packet,packetLength,0,NULL,0,0,&MediaServer_SendEventSink,info,upnptoken);
}
void MediaServer_SendEvent(void *upnptoken, char* body, const int bodylength, const char* eventname)
{
   struct SubscriberInfo *info = NULL;
   struct MediaServer_DataObject* MediaServer_Object = (struct MediaServer_DataObject*)upnptoken;
   struct sockaddr_in dest;
   LVL3DEBUG(struct timeval tv;)
   
   if(MediaServer_Object==NULL)
   {
      free(body);
      return;
   }
   sem_wait(&(MediaServer_Object->EventLock));
   if(strncmp(eventname,"ConnectionManager",17)==0)
	{
		info = MediaServer_Object->HeadSubscriberPtr_ConnectionManager;
	}
	if(strncmp(eventname,"ContentDirectory",16)==0)
	{
		info = MediaServer_Object->HeadSubscriberPtr_ContentDirectory;
	}

   memset(&dest,0,sizeof(dest));
   while(info!=NULL)
   {
      if(!MediaServer_SubscriptionExpired(info))
      {
         MediaServer_SendEvent_Body(upnptoken,body,bodylength,info);
      }
      else
      {
         //Remove Subscriber
         LVL3DEBUG(gettimeofday(&tv,NULL);)
         LVL3DEBUG(printf("\r\n\r\nTIMESTAMP: %d\r\n",tv.tv_sec);)
         LVL3DEBUG(printf("Did not renew [%s] %d.%d.%d.%d:%d UNSUBSCRIBING <%d>\r\n\r\n",((struct SubscriberInfo*)info)->SID,(((struct SubscriberInfo*)info)->Address&0xFF),((((struct SubscriberInfo*)info)->Address>>8)&0xFF),((((struct SubscriberInfo*)info)->Address>>16)&0xFF),((((struct SubscriberInfo*)info)->Address>>24)&0xFF),((struct SubscriberInfo*)info)->Port,info);)
      }
      
      info = info->Next;
   }
   
   sem_post(&(MediaServer_Object->EventLock));
}

/*! \fn MediaServer_SetState_ConnectionManager_SourceProtocolInfo(MediaServer_MicroStackToken upnptoken, char* val)
	\brief Sets the state of SourceProtocolInfo << urn:schemas-upnp-org:service:ConnectionManager:1 << ConnectionManager \par
	\b Note: Must be called at least once prior to start
	\param upnptoken The MicroStack token
	\param val The new value of the state variable
*/
void MediaServer_SetState_ConnectionManager_SourceProtocolInfo(MediaServer_MicroStackToken upnptoken, char* val)
{
	struct MediaServer_DataObject *MediaServer_Object = (struct MediaServer_DataObject*)upnptoken;
  char* body;
  int bodylength;
  char* valstr;
  valstr = (char*)malloc(ILibXmlEscapeLength(val)+1);
  ILibXmlEscape(valstr,val);
  if (MediaServer_Object->ConnectionManager_SourceProtocolInfo != NULL) free(MediaServer_Object->ConnectionManager_SourceProtocolInfo);
  MediaServer_Object->ConnectionManager_SourceProtocolInfo = valstr;
  body = (char*)malloc(46 + (int)strlen(valstr));
  bodylength = sprintf(body,"%s>%s</%s","SourceProtocolInfo",valstr,"SourceProtocolInfo");
  MediaServer_SendEvent(upnptoken,body,bodylength,"ConnectionManager");
  free(body);
}

/*! \fn MediaServer_SetState_ConnectionManager_SinkProtocolInfo(MediaServer_MicroStackToken upnptoken, char* val)
	\brief Sets the state of SinkProtocolInfo << urn:schemas-upnp-org:service:ConnectionManager:1 << ConnectionManager \par
	\b Note: Must be called at least once prior to start
	\param upnptoken The MicroStack token
	\param val The new value of the state variable
*/
void MediaServer_SetState_ConnectionManager_SinkProtocolInfo(MediaServer_MicroStackToken upnptoken, char* val)
{
	struct MediaServer_DataObject *MediaServer_Object = (struct MediaServer_DataObject*)upnptoken;
  char* body;
  int bodylength;
  char* valstr;
  valstr = (char*)malloc(ILibXmlEscapeLength(val)+1);
  ILibXmlEscape(valstr,val);
  if (MediaServer_Object->ConnectionManager_SinkProtocolInfo != NULL) free(MediaServer_Object->ConnectionManager_SinkProtocolInfo);
  MediaServer_Object->ConnectionManager_SinkProtocolInfo = valstr;
  body = (char*)malloc(42 + (int)strlen(valstr));
  bodylength = sprintf(body,"%s>%s</%s","SinkProtocolInfo",valstr,"SinkProtocolInfo");
  MediaServer_SendEvent(upnptoken,body,bodylength,"ConnectionManager");
  free(body);
}

/*! \fn MediaServer_SetState_ConnectionManager_CurrentConnectionIDs(MediaServer_MicroStackToken upnptoken, char* val)
	\brief Sets the state of CurrentConnectionIDs << urn:schemas-upnp-org:service:ConnectionManager:1 << ConnectionManager \par
	\b Note: Must be called at least once prior to start
	\param upnptoken The MicroStack token
	\param val The new value of the state variable
*/
void MediaServer_SetState_ConnectionManager_CurrentConnectionIDs(MediaServer_MicroStackToken upnptoken, char* val)
{
	struct MediaServer_DataObject *MediaServer_Object = (struct MediaServer_DataObject*)upnptoken;
  char* body;
  int bodylength;
  char* valstr;
  valstr = (char*)malloc(ILibXmlEscapeLength(val)+1);
  ILibXmlEscape(valstr,val);
  if (MediaServer_Object->ConnectionManager_CurrentConnectionIDs != NULL) free(MediaServer_Object->ConnectionManager_CurrentConnectionIDs);
  MediaServer_Object->ConnectionManager_CurrentConnectionIDs = valstr;
  body = (char*)malloc(50 + (int)strlen(valstr));
  bodylength = sprintf(body,"%s>%s</%s","CurrentConnectionIDs",valstr,"CurrentConnectionIDs");
  MediaServer_SendEvent(upnptoken,body,bodylength,"ConnectionManager");
  free(body);
}

/*! \fn MediaServer_SetState_ContentDirectory_ContainerUpdateIDs(MediaServer_MicroStackToken upnptoken, char* val)
	\brief Sets the state of ContainerUpdateIDs << urn:schemas-upnp-org:service:ContentDirectory:1 << ContentDirectory \par
	\b Note: Must be called at least once prior to start
	\param upnptoken The MicroStack token
	\param val The new value of the state variable
*/
void MediaServer_SetState_ContentDirectory_ContainerUpdateIDs(MediaServer_MicroStackToken upnptoken, char* val)
{
	struct MediaServer_DataObject *MediaServer_Object = (struct MediaServer_DataObject*)upnptoken;
  char* body;
  int bodylength;
  char* valstr;
  valstr = (char*)malloc(ILibXmlEscapeLength(val)+1);
  ILibXmlEscape(valstr,val);
  if (MediaServer_Object->ContentDirectory_ContainerUpdateIDs != NULL) free(MediaServer_Object->ContentDirectory_ContainerUpdateIDs);
  MediaServer_Object->ContentDirectory_ContainerUpdateIDs = valstr;
  body = (char*)malloc(46 + (int)strlen(valstr));
  bodylength = sprintf(body,"%s>%s</%s","ContainerUpdateIDs",valstr,"ContainerUpdateIDs");
  MediaServer_SendEvent(upnptoken,body,bodylength,"ContentDirectory");
  free(body);
}

/*! \fn MediaServer_SetState_ContentDirectory_SystemUpdateID(MediaServer_MicroStackToken upnptoken, unsigned int val)
	\brief Sets the state of SystemUpdateID << urn:schemas-upnp-org:service:ContentDirectory:1 << ContentDirectory \par
	\b Note: Must be called at least once prior to start
	\param upnptoken The MicroStack token
	\param val The new value of the state variable
*/
void MediaServer_SetState_ContentDirectory_SystemUpdateID(MediaServer_MicroStackToken upnptoken, unsigned int val)
{
	struct MediaServer_DataObject *MediaServer_Object = (struct MediaServer_DataObject*)upnptoken;
  char* body;
  int bodylength;
  char* valstr;
  valstr = (char*)malloc(10);
  sprintf(valstr,"%u",val);
  if (MediaServer_Object->ContentDirectory_SystemUpdateID != NULL) free(MediaServer_Object->ContentDirectory_SystemUpdateID);
  MediaServer_Object->ContentDirectory_SystemUpdateID = valstr;
  body = (char*)malloc(38 + (int)strlen(valstr));
  bodylength = sprintf(body,"%s>%s</%s","SystemUpdateID",valstr,"SystemUpdateID");
  MediaServer_SendEvent(upnptoken,body,bodylength,"ContentDirectory");
  free(body);
}



void MediaServer_DestroyMicroStack(void *object)
{
   struct MediaServer_DataObject *upnp = (struct MediaServer_DataObject*)object;
   struct SubscriberInfo  *sinfo,*sinfo2;
   MediaServer_SendByeBye(upnp);
   
   sem_destroy(&(upnp->EventLock));
   
   free(upnp->ConnectionManager_SourceProtocolInfo);
	free(upnp->ConnectionManager_SinkProtocolInfo);
	free(upnp->ConnectionManager_CurrentConnectionIDs);
	free(upnp->ContentDirectory_ContainerUpdateIDs);
	free(upnp->ContentDirectory_SystemUpdateID);

   
   free(upnp->AddressList);
   free(upnp->NOTIFY_SEND_socks);
   free(upnp->NOTIFY_RECEIVE_socks);
   free(upnp->UUID);
   free(upnp->Serial);
   
   
   sinfo = upnp->HeadSubscriberPtr_ConnectionManager;
	while(sinfo!=NULL)
	{
		sinfo2 = sinfo->Next;
		MediaServer_DestructSubscriberInfo(sinfo);
		sinfo = sinfo2;
	}
	sinfo = upnp->HeadSubscriberPtr_ContentDirectory;
	while(sinfo!=NULL)
	{
		sinfo2 = sinfo->Next;
		MediaServer_DestructSubscriberInfo(sinfo);
		sinfo = sinfo2;
	}

   
}
int MediaServer_GetLocalPortNumber(MediaServer_SessionToken token)
{
   return(ILibWebServer_GetPortNumber(((struct ILibWebServer_Session*)token)->Parent));
}
void MediaServer_SessionReceiveSink(
struct ILibWebServer_Session *sender,
int InterruptFlag,
struct packetheader *header,
char *bodyBuffer,
int *beginPointer,
int endPointer,
int done)
{
   
   char *txt;
   if(header!=NULL && sender->User3==NULL && done==0)
   {
      sender->User3 = (void*)~0;
      txt = ILibGetHeaderLine(header,"Expect",6);
      if(txt!=NULL)
      {
         if(strcasecmp(txt,"100-Continue")==0)
         {
            //
            // Expect Continue
            //
            ILibWebServer_Send_Raw(sender,"HTTP/1.1 100 Continue\r\n\r\n",25,ILibAsyncSocket_MemoryOwnership_STATIC,0);
         }
         else
         {
            //
            // Don't understand
            //
            ILibWebServer_Send_Raw(sender,"HTTP/1.1 417 Expectation Failed\r\n\r\n",35,ILibAsyncSocket_MemoryOwnership_STATIC,1);
            ILibWebServer_DisconnectSession(sender);
            return;
         }
      }
   }
   
   if(header!=NULL && done !=0 && InterruptFlag==0)
   {
      MediaServer_ProcessHTTPPacket(sender,header,bodyBuffer,beginPointer==NULL?0:*beginPointer,endPointer);
      if(beginPointer!=NULL) {*beginPointer = endPointer;}
   }
}
void MediaServer_SessionSink(struct ILibWebServer_Session *SessionToken, void *user)
{
   SessionToken->OnReceive = &MediaServer_SessionReceiveSink;
   SessionToken->User = user;
}
void MediaServer_SetTag(const MediaServer_MicroStackToken token, void *UserToken)
{
   ((struct MediaServer_DataObject*)token)->User = UserToken;
}
void *MediaServer_GetTag(const MediaServer_MicroStackToken token)
{
   return(((struct MediaServer_DataObject*)token)->User);
}
MediaServer_MicroStackToken MediaServer_GetMicroStackTokenFromSessionToken(const MediaServer_SessionToken token)
{
   return(((struct ILibWebServer_Session*)token)->User);
}
MediaServer_MicroStackToken MediaServer_CreateMicroStack(void *Chain, const char* FriendlyName,const char* UDN, const char* SerialNumber, const int NotifyCycleSeconds, const unsigned short PortNum)

{
   struct MediaServer_DataObject* RetVal = (struct MediaServer_DataObject*)malloc(sizeof(struct MediaServer_DataObject));
   
   
   struct timeval tv;
   gettimeofday(&tv,NULL);
   srand((int)tv.tv_sec);
   
   MediaServer__Device_MediaServer_Impl.FriendlyName = FriendlyName;
	MediaServer__Device_MediaServer_Impl.UDN = UDN;
	MediaServer__Device_MediaServer_Impl.Serial = SerialNumber;
	if(MediaServer__Device_MediaServer_Impl.Manufacturer == NULL) {MediaServer__Device_MediaServer_Impl.Manufacturer = "Intel's Connected and Extended PC Lab";}
   if(MediaServer__Device_MediaServer_Impl.ManufacturerURL == NULL) {MediaServer__Device_MediaServer_Impl.ManufacturerURL = "http://www.intel.com";}
   if(MediaServer__Device_MediaServer_Impl.ModelDescription == NULL) {MediaServer__Device_MediaServer_Impl.ModelDescription = "UPnP/AV 1.0 Compliant Media Server";}
   if(MediaServer__Device_MediaServer_Impl.ModelName == NULL) {MediaServer__Device_MediaServer_Impl.ModelName = "Digital Media Server";}
   if(MediaServer__Device_MediaServer_Impl.ModelNumber == NULL) {MediaServer__Device_MediaServer_Impl.ModelNumber = "None";}
   if(MediaServer__Device_MediaServer_Impl.ModelURL == NULL) {MediaServer__Device_MediaServer_Impl.ModelURL = "http://www.intel.com";}
   if(MediaServer__Device_MediaServer_Impl.ProductCode == NULL) {MediaServer__Device_MediaServer_Impl.ProductCode = "None";}
   
   
   /* Complete State Reset */
   memset(RetVal,0,sizeof(struct MediaServer_DataObject));
   
   RetVal->ForceExit = 0;
   RetVal->PreSelect = &MediaServer_MasterPreSelect;
   RetVal->PostSelect = NULL;
   RetVal->Destroy = &MediaServer_DestroyMicroStack;
   RetVal->InitialNotify = 0;
   if (UDN != NULL)
   {
      RetVal->UUID = (char*)malloc((int)strlen(UDN)+6);
      sprintf(RetVal->UUID,"uuid:%s",UDN);
      RetVal->UDN = RetVal->UUID + 5;
   }
   if (SerialNumber != NULL)
   {
      RetVal->Serial = (char*)malloc((int)strlen(SerialNumber)+1);
      strcpy(RetVal->Serial,SerialNumber);
   }
   
   
   
   RetVal->WebServerTimer = ILibCreateLifeTime(Chain);
   
   RetVal->HTTPServer = ILibWebServer_Create(Chain,UPNP_HTTP_MAXSOCKETS,PortNum,&MediaServer_SessionSink,RetVal);
   RetVal->WebSocketPortNumber=(int)ILibWebServer_GetPortNumber(RetVal->HTTPServer);
   
   
   
   ILibAddToChain(Chain,RetVal);
   MediaServer_Init(RetVal,Chain,NotifyCycleSeconds,PortNum);
   
   RetVal->EventClient = ILibCreateWebClient(5,Chain);
   RetVal->UpdateFlag = 0;
   
   
   
   sem_init(&(RetVal->EventLock),0,1);
   return(RetVal);
}






void MediaServer_StreamDescriptionDocument(struct ILibWebServer_Session *session)
{
   #if defined(WIN32) || defined(_WIN32_WCE)
   char *responseHeader = "\r\nCONTENT-TYPE:  text/xml; charset=\"utf-8\"\r\nServer: WINDOWS, UPnP/1.0, Intel MicroStack/1.0.2384";
   #else
   char *responseHeader = "\r\nCONTENT-TYPE:  text/xml; charset=\"utf-8\"\r\nServer: POSIX, UPnP/1.0, Intel MicroStack/1.0.2384";
   #endif
   
   char *tempString;
   int tempStringLength;
   char *xString,*xString2;
   
   
   //
   // Device
   //
   ILibWebServer_StreamHeader_Raw(session,200,"OK",responseHeader,1);
   
   
   
   xString2 = ILibDecompressString(MediaServer__Device_MediaServer_Impl.Reserved,MediaServer__Device_MediaServer_Impl.ReservedXL,MediaServer__Device_MediaServer_Impl.ReservedUXL);
   xString = ILibString_Replace(xString2,(int)strlen(xString2),"http://255.255.255.255:255/",27,"%s",2);
   free(xString2);
   tempStringLength = (int)(strlen(xString)+strlen(MediaServer__Device_MediaServer_Impl.Manufacturer)+strlen(MediaServer__Device_MediaServer_Impl.ManufacturerURL)+strlen(MediaServer__Device_MediaServer_Impl.ModelDescription)+strlen(MediaServer__Device_MediaServer_Impl.ModelName)+strlen(MediaServer__Device_MediaServer_Impl.ModelNumber)+strlen(MediaServer__Device_MediaServer_Impl.ModelURL)+strlen(MediaServer__Device_MediaServer_Impl.ProductCode)+strlen(MediaServer__Device_MediaServer_Impl.FriendlyName)+strlen(MediaServer__Device_MediaServer_Impl.UDN)+strlen(MediaServer__Device_MediaServer_Impl.Serial));
   tempString = (char*)malloc(tempStringLength);
   tempStringLength = sprintf(tempString,xString,
   MediaServer__Device_MediaServer_Impl.FriendlyName,
   MediaServer__Device_MediaServer_Impl.Manufacturer,
   MediaServer__Device_MediaServer_Impl.ManufacturerURL,
   MediaServer__Device_MediaServer_Impl.ModelDescription,
   MediaServer__Device_MediaServer_Impl.ModelName,
   MediaServer__Device_MediaServer_Impl.ModelNumber,
   MediaServer__Device_MediaServer_Impl.ModelURL,
   MediaServer__Device_MediaServer_Impl.Serial,
   MediaServer__Device_MediaServer_Impl.UDN);
   free(xString);
   ILibWebServer_StreamBody(session,tempString,tempStringLength-19,ILibAsyncSocket_MemoryOwnership_CHAIN,0);
   
   //
   // Embedded Services
   //
   ILibWebServer_StreamBody(session,"<serviceList>",13,ILibAsyncSocket_MemoryOwnership_STATIC,0);
   
   if(MediaServer__Device_MediaServer_Impl.ConnectionManager!=NULL)
   {
      xString = ILibDecompressString(MediaServer__Device_MediaServer_Impl.ConnectionManager->Reserved,MediaServer__Device_MediaServer_Impl.ConnectionManager->ReservedXL,MediaServer__Device_MediaServer_Impl.ConnectionManager->ReservedUXL);
      ILibWebServer_StreamBody(session,xString,MediaServer__Device_MediaServer_Impl.ConnectionManager->ReservedUXL,ILibAsyncSocket_MemoryOwnership_CHAIN,0);
   }
   
   if(MediaServer__Device_MediaServer_Impl.ContentDirectory!=NULL)
   {
      xString = ILibDecompressString(MediaServer__Device_MediaServer_Impl.ContentDirectory->Reserved,MediaServer__Device_MediaServer_Impl.ContentDirectory->ReservedXL,MediaServer__Device_MediaServer_Impl.ContentDirectory->ReservedUXL);
      ILibWebServer_StreamBody(session,xString,MediaServer__Device_MediaServer_Impl.ContentDirectory->ReservedUXL,ILibAsyncSocket_MemoryOwnership_CHAIN,0);
   }
   
   ILibWebServer_StreamBody(session,"</serviceList>",14,ILibAsyncSocket_MemoryOwnership_STATIC,0);
   
   ILibWebServer_StreamBody(session,"</device>",9,ILibAsyncSocket_MemoryOwnership_STATIC,0);
   
   
   ILibWebServer_StreamBody(session,"</root>",7,ILibAsyncSocket_MemoryOwnership_STATIC,1);
}
struct MediaServer__Device_MediaServer* MediaServer_GetConfiguration()
{
	return(&(MediaServer__Device_MediaServer_Impl));
}



