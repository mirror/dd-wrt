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
 * $Workfile: DLNAProtocolInfo.c
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
#include <malloc.h>
#include "DLNAProtocolInfo.h"
#include "ILibParsers.h"


struct DLNA_Flags
{
	unsigned int Bit0  :1 ;
	unsigned int Bit1  :1 ;
	unsigned int Bit2  :1 ;
	unsigned int Bit3  :1 ;
	unsigned int Bit4  :1 ;
	unsigned int Bit5  :1 ;
	unsigned int Bit6  :1 ;
	unsigned int Bit7  :1 ;
	unsigned int Bit8  :1 ;
	unsigned int Bit9  :1 ;
	unsigned int Bit10 :1 ;
	unsigned int Bit11 :1 ;
	unsigned int Bit12 :1 ;
	unsigned int Bit13 :1 ;
	unsigned int Bit14 :1 ;
	unsigned int Bit15 :1 ;
	unsigned int Bit16 :1 ;
	unsigned int Bit17 :1 ;
	unsigned int Bit18 :1 ;
	unsigned int Bit19 :1 ;
	unsigned int Bit20 :1 ;
	unsigned int Bit21 :1 ;
	unsigned int Bit22 :1 ;
	unsigned int Bit23 :1 ;
	unsigned int Bit24 :1 ;
	unsigned int Bit25 :1 ;
	unsigned int Bit26 :1 ;
	unsigned int Bit27 :1 ;
	unsigned int Bit28 :1 ;
	unsigned int Bit29 :1 ;
	unsigned int Bit30 :1 ;
	unsigned int Bit31 :1 ;
};

typedef struct DLNA_Flags* DLNA_FLAG_BITS;

DLNA_FLAG_BITS DLNA_Bits(unsigned long *number)
{
	DLNA_FLAG_BITS RetVal = (DLNA_FLAG_BITS)number;
	return(RetVal);
}

unsigned long DLNA_StringToLong(int numberBase,char *buffer, int length)
{
	char buf[9];
	char *ep;
	unsigned long RetVal;

	memset(buf,0,9);
	memcpy(buf,buffer,length<=8?length:8);
	
	RetVal = strtoul(buf,&ep,numberBase);
	return(RetVal);
}
char* DLNA_LongToString(unsigned long inValue, int numberOfDigits)
{
	char *RetVal = (char*)malloc(64);
	int length;
	int padding;
	
	memset(RetVal,0,64);
	length = sprintf(RetVal,"%lX",inValue);
	padding = numberOfDigits-length;

	if(padding>0)
	{
		memmove(RetVal+padding,RetVal,length);
		memset(RetVal,'0',padding);
	}
	else if(padding<0)
	{
		memmove(RetVal,RetVal+length-numberOfDigits,numberOfDigits);
		memset(RetVal+numberOfDigits,0,64-numberOfDigits);
	}
	return(RetVal);
}
struct DLNAProtocolInfo *DLNAProtocolInfo_Parse(char *protocolInfo, int protocolInfoLength)
{
	struct parser_result *pr,*pr2,*pr3,*pr4;
	struct parser_result_field *prf,*prf2;
	char *temp;

	long flags;
	int tempInt;

	struct DLNAProtocolInfo *RetVal = (struct DLNAProtocolInfo*)malloc(sizeof(struct DLNAProtocolInfo));
	memset(RetVal,0,sizeof(struct DLNAProtocolInfo));
	RetVal->NameValueTable = ILibInitHashTree_CaseInSensitive();

	RetVal->DLNA_Major_Version = 1;
	RetVal->Protocol = DLNAProtocolInfo_ProtocolType_UNKNOWN;

	pr = ILibParseString(protocolInfo,0,protocolInfoLength,":",1);
	if(pr->NumResults==4 || pr->NumResults==1)
	{
		if(pr->NumResults==4)
		{
			if(pr->FirstResult->datalength==8 && memcmp(pr->FirstResult->data,"http-get",8)==0)
			{
				RetVal->Protocol = DLNAProtocolInfo_ProtocolType_HTTP;
			}
			else if(pr->FirstResult->datalength==19 && memcmp(pr->FirstResult->data,"playsingle-http-get",19)==0)
			{
				RetVal->Protocol = DLNAProtocolInfo_ProtocolType_HTTP;
				RetVal->IsPlaySingleUri = 1;
			}
			else if(pr->FirstResult->datalength==12 && memcmp(pr->FirstResult->data,"rtsp-rtp-udp",12)==0)
			{
				RetVal->Protocol = DLNAProtocolInfo_ProtocolType_RTP;
			}
			else if(pr->FirstResult->datalength==23 && memcmp(pr->FirstResult->data,"playsingle-rtsp-rtp-udp",23)==0)
			{
				RetVal->Protocol = DLNAProtocolInfo_ProtocolType_RTP;
				RetVal->IsPlaySingleUri = 1;
			}
			else if(pr->FirstResult->datalength==1 && memcmp(pr->FirstResult->data,"*",1)==0)
			{
				RetVal->Protocol = DLNAProtocolInfo_ProtocolType_ANY;
			}

			RetVal->MimeType = (char*)malloc(pr->FirstResult->NextResult->NextResult->datalength+1);
			memcpy(RetVal->MimeType,pr->FirstResult->NextResult->NextResult->data,pr->FirstResult->NextResult->NextResult->datalength);
			RetVal->MimeType[pr->FirstResult->NextResult->NextResult->datalength]=0;
		}
		pr2 = ILibParseString(pr->LastResult->data,0,pr->LastResult->datalength,";",1);
		prf = pr2->FirstResult;
		while(prf!=NULL)
		{
			pr3 = ILibParseString(prf->data,0,prf->datalength,"=",1);
			ILibAddEntry(RetVal->NameValueTable,pr3->FirstResult->data,pr3->FirstResult->datalength,ILibString_Copy(pr3->LastResult->data,pr3->LastResult->datalength));
			if(pr3->FirstResult->datalength==11 && memcmp(pr3->FirstResult->data,"DLNA.ORG_PN",11)==0)
			{
				//
				// Profile
				//
				temp = (char*)malloc(pr3->LastResult->datalength+1);
				memcpy(temp,pr3->LastResult->data,pr3->LastResult->datalength);
				temp[pr3->LastResult->datalength]=0;
				RetVal->Profile = temp;
			}
			if(pr3->FirstResult->datalength==11 && memcmp(pr3->FirstResult->data,"DLNA.ORG_OP",11)==0)
			{
				//
				// OP Code
				//
				flags = DLNA_StringToLong(2,pr3->LastResult->data,pr3->LastResult->datalength);
				if(RetVal->Protocol == DLNAProtocolInfo_ProtocolType_HTTP)
				{
					RetVal->SupportsTimeBasedSeek = DLNA_Bits(&flags)->Bit1; 
					RetVal->SupportsByteBasedSeek = DLNA_Bits(&flags)->Bit0;
				}
				else if(RetVal->Protocol == DLNAProtocolInfo_ProtocolType_RTP)
				{
					RetVal->SupportsTimeBasedSeek = DLNA_Bits(&flags)->Bit0;
				}
			}
			if(pr3->FirstResult->datalength==11 && memcmp(pr3->FirstResult->data,"DLNA.ORG_PS",11)==0)
			{
				//
				// Supported Play Speeds
				//
				pr4 = ILibParseString(pr3->LastResult->data,0,pr3->LastResult->datalength,",",1);
				RetVal->SupportedPlaySpeedsLength = pr4->NumResults;
				RetVal->SupportedPlaySpeeds = (char**)malloc(RetVal->SupportedPlaySpeedsLength*sizeof(char*));
				tempInt = 0;
				prf2 = pr4->FirstResult;
				while(prf2!=NULL)
				{
					RetVal->SupportedPlaySpeeds[tempInt] = (char*)malloc(2+prf2->datalength);
					memcpy(RetVal->SupportedPlaySpeeds[tempInt],prf2->data,prf2->datalength);
					RetVal->SupportedPlaySpeeds[tempInt][prf2->datalength]=0;
					++tempInt;
					prf2 = prf2->NextResult;
				}
				ILibDestructParserResults(pr4);
			}
			if(pr3->FirstResult->datalength==11 && memcmp(pr3->FirstResult->data,"DLNA.ORG_CI",11)==0)
			{
				//
				// Conversion Indication
				//
				RetVal->IsConvertedContent = (int)DLNA_StringToLong(2,pr3->LastResult->data,pr3->LastResult->datalength);
			}
			if(pr3->FirstResult->datalength==14 && memcmp(pr3->FirstResult->data,"DLNA.ORG_FLAGS",14)==0)
			{
				//
				// Primary Flags (8 digits)
				//
				flags = DLNA_StringToLong(16,pr3->LastResult->data,pr3->LastResult->datalength<8?pr3->LastResult->datalength:8);
				if(DLNA_Bits(&flags)->Bit20)
				{
					RetVal->DLNA_Major_Version = 1;
					RetVal->DLNA_Minor_Version = 5;
				}
				if(RetVal->DLNA_Major_Version==1 && RetVal->DLNA_Minor_Version>=5)
				{
					RetVal->SenderPaced = DLNA_Bits(&flags)->Bit31;
					RetVal->LimitedOperations_TimeBasedSeek = DLNA_Bits(&flags)->Bit30;
					RetVal->LimitedOperations_ByteBasedSeek = DLNA_Bits(&flags)->Bit29;
					RetVal->DLNAPlayContainer = DLNA_Bits(&flags)->Bit28;
					RetVal->S0_Increasing = DLNA_Bits(&flags)->Bit27;
					RetVal->SN_Increasing = DLNA_Bits(&flags)->Bit26;
					RetVal->RTSP_Pause = DLNA_Bits(&flags)->Bit25;
					RetVal->TM_S = DLNA_Bits(&flags)->Bit24;
					RetVal->TM_I = DLNA_Bits(&flags)->Bit23;
					RetVal->TM_B = DLNA_Bits(&flags)->Bit22;
				}
				RetVal->HTTP_Stalling = DLNA_Bits(&flags)->Bit21;
			}
			if(pr3->FirstResult->datalength==14 && memcmp(pr3->FirstResult->data,"DLNA.ORG_MAXSP",14)==0)
			{
				pr4 = ILibParseString(pr3->LastResult->data,0,pr3->LastResult->datalength,".",1);
				RetVal->MaxSpeed_Major = (int)DLNA_StringToLong(10,pr4->FirstResult->data,pr4->FirstResult->datalength);
				if(pr4->NumResults==2)
				{
					RetVal->MaxSpeed_Minor = (int)DLNA_StringToLong(10,pr4->LastResult->data,pr4->LastResult->datalength);
				}
				ILibDestructParserResults(pr4);
			}

			ILibDestructParserResults(pr3);
			prf = prf->NextResult;
		}
		ILibDestructParserResults(pr2);
	}
	ILibDestructParserResults(pr);
	return(RetVal);
}
void DLNAProtocolInfo_Destruct(struct DLNAProtocolInfo *protocolInfo)
{
	int i;
	void *en;
	void *data;
	char *key;
	int keyLength;

	if(protocolInfo->SupportedPlaySpeedsLength>0)
	{
		for(i=0;i<protocolInfo->SupportedPlaySpeedsLength;++i)
		{
			if(protocolInfo->SupportedPlaySpeeds[i] != NULL)
			{
				free(protocolInfo->SupportedPlaySpeeds[i]);
			}
		}
		free(protocolInfo->SupportedPlaySpeeds); 
	}

	if(protocolInfo->MimeType!=NULL)
	{
		free(protocolInfo->MimeType);
	}
	if(protocolInfo->Profile!=NULL)
	{
		free(protocolInfo->Profile);
	}
	if(protocolInfo->NameValueTable!=NULL)
	{
		en = ILibHashTree_GetEnumerator(protocolInfo->NameValueTable);
		while(ILibHashTree_MoveNext(en)==0)
		{
			ILibHashTree_GetValue(en,&key,&keyLength,&data);
			free(data);
		}
		ILibDestroyHashTree(protocolInfo->NameValueTable);
	}
	free(protocolInfo);
}
void DLNA_GetContentInfo(ILibWebClient_RequestManager request_manager, char *contentURI, void *user, DLNA_OnGetContentInfo OnGetContentInfo)
{
}
char *DLNAProtocolInfo_Serialize(struct DLNAProtocolInfo *protocolInfo)
{
	char *RetVal;
	int RetValLength = 0;
	unsigned long flags = 0;
	int i;
	void *en;
	char *key;
	int keyLength;
	void *data;

	// http-get:*:audio/mpeg:DLNA.ORG_PN=MPEG_PS_NTSC;DLNA.ORG_OP=01;DLNA.ORG_FLAGS=05100000000000000000000000000000

	switch(protocolInfo->Protocol)
	{
		case DLNAProtocolInfo_ProtocolType_RTP:
			RetValLength += 12;
			break;
		case DLNAProtocolInfo_ProtocolType_HTTP:
			RetValLength += 8;
			break;
		default:
			break;
	}
	RetValLength += 3; // :*:
	if(protocolInfo->MimeType!=NULL)
	{
		RetValLength += (int)strlen(protocolInfo->MimeType);
	}
	RetValLength += 1; // :

	if(protocolInfo->SupportedPlaySpeedsLength>0)
	{
		RetValLength += 12; // DLNA.ORG_PS=
		for(i=0;i<protocolInfo->SupportedPlaySpeedsLength;++i)
		{
			if(i!=0)
			{
				RetValLength += 1; // ,
			}
			RetValLength += (int)strlen(protocolInfo->SupportedPlaySpeeds[i]);
		}
		RetValLength += 1; //  ;
	}

	if(protocolInfo->IsConvertedContent!=0)
	{
		RetValLength += 14; // DLNA.ORG_CI=1;
	}

	if(protocolInfo->MaxSpeed_Major+protocolInfo->MaxSpeed_Minor!=0)
	{
		RetValLength += 32; //DLNA.ORG_MAXSP=xxx.xxx;
	}


	//
	// Fourth Field Stuff
	//
	if(protocolInfo->Profile!=NULL)
	{
		RetValLength += (13 + (int)strlen(protocolInfo->Profile));
	}
	RetValLength += 15; // DLNA.ORG_OP=01;
	RetValLength += 48; // DLNA.ORG_FLAGS=05100000000000000000000000000000

	if(protocolInfo->NameValueTable!=NULL)
	{
		en = ILibHashTree_GetEnumerator(protocolInfo->NameValueTable);
		while(ILibHashTree_MoveNext(en)==0)
		{
			ILibHashTree_GetValue(en,&key,&keyLength,&data);
			if( !(keyLength==14&&memcmp(key,"DLNA.ORG_FLAGS",keyLength)==0) &&
				!(keyLength==14&&memcmp(key,"DLNA.ORG_MAXSP",keyLength)==0) &&
				!(keyLength==11&&memcmp(key,"DLNA.ORG_CI",keyLength)==0) &&
				!(keyLength==11&&memcmp(key,"DLNA.ORG_PS",keyLength)==0) &&
				!(keyLength==11&&memcmp(key,"DLNA.ORG_OP",keyLength)==0) &&
				!(keyLength==11&&memcmp(key,"DLNA.ORG_PN",keyLength)==0) )
			{
				RetValLength += (keyLength+(int)strlen((char*)data)+2); // key=value;
			}
		}
		ILibHashTree_DestroyEnumerator(en);
	}


	//
	// Lets actually serialize the structure
	//

	RetVal = (char*)malloc(RetValLength+1);
	memset(RetVal,0,RetValLength+1);
	RetValLength = 0;

	switch(protocolInfo->Protocol)
	{
		case DLNAProtocolInfo_ProtocolType_RTP:
			RetValLength = sprintf(RetVal,"rtsp-rtp-udp");
			break;
		case DLNAProtocolInfo_ProtocolType_HTTP:
			RetValLength = sprintf(RetVal,"http-get");
			break;
		default:
			break;	
	}
	RetValLength += sprintf(RetVal+RetValLength,":*:");
	RetValLength += sprintf(RetVal+RetValLength,"%s:",protocolInfo->MimeType);

	//
	// Fourth Field Stuff
	//
	if(protocolInfo->Profile!=NULL)
	{
		RetValLength += sprintf(RetVal+RetValLength,"DLNA.ORG_PN=%s;",protocolInfo->Profile);
	}
	switch(protocolInfo->Protocol)
	{
		case DLNAProtocolInfo_ProtocolType_RTP:
			RetValLength += sprintf(RetVal+RetValLength,"DLNA.ORG_OP=0%d;",protocolInfo->SupportsTimeBasedSeek);
			break;
		case DLNAProtocolInfo_ProtocolType_HTTP:
			RetValLength += sprintf(RetVal+RetValLength,"DLNA.ORG_OP=%d%d;",protocolInfo->SupportsTimeBasedSeek,protocolInfo->SupportsByteBasedSeek);
			break;	
		default:
			break;
	}

	//
	// Supported Play Speeds
	//
	if(protocolInfo->SupportedPlaySpeedsLength>0)
	{
		RetValLength += sprintf(RetVal+RetValLength,"DLNA.ORG_PS=");
		for(i=0;i<protocolInfo->SupportedPlaySpeedsLength;++i)
		{
			if(i!=0)
			{
				RetValLength += sprintf(RetVal+RetValLength,",");
			}
			RetValLength += sprintf(RetVal+RetValLength,"%s",protocolInfo->SupportedPlaySpeeds[i]);
		}
		RetValLength += sprintf(RetVal+RetValLength,";");
	}


	//
	// Conversion Indication
	//
	if(protocolInfo->IsConvertedContent!=0)
	{
		RetValLength += sprintf(RetVal+RetValLength,"DLNA.ORG_CI=1;");
	}
	
	//
	// Max Speed
	//
	if(protocolInfo->MaxSpeed_Major + protocolInfo->MaxSpeed_Minor != 0)
	{
		RetValLength += sprintf(RetVal+RetValLength,"DLNA.ORG_MAXSP=%d.%d;",protocolInfo->MaxSpeed_Major,protocolInfo->MaxSpeed_Minor);
	}
	

	if(protocolInfo->DLNA_Major_Version==1 && protocolInfo->DLNA_Minor_Version==5)
	{
		DLNA_Bits(&flags)->Bit20 = 1;
	}
	if(protocolInfo->DLNA_Major_Version==1 && protocolInfo->DLNA_Minor_Version>=5)
	{
		DLNA_Bits(&flags)->Bit31 = protocolInfo->SenderPaced;
		DLNA_Bits(&flags)->Bit30 = protocolInfo->LimitedOperations_TimeBasedSeek;
		DLNA_Bits(&flags)->Bit29 = protocolInfo->LimitedOperations_ByteBasedSeek;
		DLNA_Bits(&flags)->Bit28 = protocolInfo->DLNAPlayContainer;
		DLNA_Bits(&flags)->Bit27 = protocolInfo->S0_Increasing;
		DLNA_Bits(&flags)->Bit26 = protocolInfo->SN_Increasing;
		DLNA_Bits(&flags)->Bit25 = protocolInfo->RTSP_Pause;
		DLNA_Bits(&flags)->Bit24 = protocolInfo->TM_S;
		DLNA_Bits(&flags)->Bit23 = protocolInfo->TM_I;
		DLNA_Bits(&flags)->Bit22 = protocolInfo->TM_B;
		DLNA_Bits(&flags)->Bit21 = protocolInfo->HTTP_Stalling;
	}
	RetValLength += sprintf(RetVal+RetValLength,"DLNA.ORG_FLAGS=%08lX%024d;",flags,0);

	if(protocolInfo->NameValueTable!=NULL)
	{
		en = ILibHashTree_GetEnumerator(protocolInfo->NameValueTable);
		while(ILibHashTree_MoveNext(en)==0)
		{
			ILibHashTree_GetValue(en,&key,&keyLength,&data);
			if( !(keyLength==14&&memcmp(key,"DLNA.ORG_FLAGS",keyLength)==0) &&
				!(keyLength==14&&memcmp(key,"DLNA.ORG_MAXSP",keyLength)==0) &&
				!(keyLength==11&&memcmp(key,"DLNA.ORG_CI",keyLength)==0) &&
				!(keyLength==11&&memcmp(key,"DLNA.ORG_PS",keyLength)==0) &&
				!(keyLength==11&&memcmp(key,"DLNA.ORG_OP",keyLength)==0) &&
				!(keyLength==11&&memcmp(key,"DLNA.ORG_PN",keyLength)==0) )
			{
				key[keyLength]=0;
				RetValLength += sprintf(RetVal+RetValLength,"%s=%s;",key,(char*)data);
			}
		}
		ILibHashTree_DestroyEnumerator(en);
	}


	if(RetVal[RetValLength-1]==';')
	{
		RetVal[RetValLength-1]=0;
	}
	return(RetVal);
}
int DLNAProtocolInfo_IsMatch(struct DLNAProtocolInfo *source, struct DLNAProtocolInfo *target)
{
	int RetVal = 0;
	if(source->Protocol==target->Protocol || 
		source->Protocol==DLNAProtocolInfo_ProtocolType_ANY ||
		target->Protocol==DLNAProtocolInfo_ProtocolType_ANY)
	{
		if(strcasecmp(source->MimeType,target->MimeType)==0)
		{
			if(source->Profile!=NULL && target->Profile!=NULL)
			{
				if(strcasecmp(source->Profile,target->Profile)==0)
				{
					//
					// Match!
					//
					RetVal=1;
				}
			}
			else
			{
				//
				// DLNA Profiles are not specified, but otherwise still matches
				//
				RetVal=1;
			}
		}
	}
	return(RetVal);
}
char *DLNAProtocolInfo_GetNameValuePair_Value(struct DLNAProtocolInfo *protocolInfo, char *Name)
{
	return((char*)ILibGetEntry(protocolInfo->NameValueTable,Name,(int)strlen(Name)));
}
