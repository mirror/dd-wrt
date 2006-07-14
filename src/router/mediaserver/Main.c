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
 * $Workfile: Main.c
 * $Revision: #1.0.2384.30305
 * $Author:   Intel Corporation, Intel Device Builder
 * $Date:     Thursday, July 13, 2006
 *
 *
 *
 */

#if defined(WIN32)
	#ifndef MICROSTACK_NO_STDAFX
		#include "stdafx.h"
	#endif
	#define _CRTDBG_MAP_ALLOC
	#include <TCHAR.h>
#endif

#if defined(WINSOCK2)
	#include <winsock2.h>
	#include <ws2tcpip.h>
#elif defined(WINSOCK1)
	#include <winsock.h>
	#include <wininet.h>
#endif

#include "ILibParsers.h"

#include "MediaServer_MicroStack.h"
#include "ILibWebServer.h"
#include "ILibAsyncSocket.h"



#include "ILibThreadPool.h"
#include <pthread.h>
#include "DmsIntegration.h"

#if defined(WIN32)
	#include <crtdbg.h>
#endif


void *MicroStackChain;






void *ILib_Pool;

void *DMSObject;




void *ILib_Monitor;
int ILib_IPAddressLength;
int *ILib_IPAddressList;
























void ILib_IPAddressMonitor(void *data)
{
	int length;
	int *list;
	
	length = ILibGetLocalIPAddressList(&list);
	if(length!=ILib_IPAddressLength || memcmp((void*)list,(void*)ILib_IPAddressList,sizeof(int)*length)!=0)
	{
		
		
DMS_NotifyIPAddressChange(DMSObject);

		
		
		
		free(ILib_IPAddressList);
		ILib_IPAddressList = list;
		ILib_IPAddressLength = length;
	}
	else
	{
		free(list);
	}
	
	
	ILibLifeTime_Add(ILib_Monitor,NULL,4,(void*)&ILib_IPAddressMonitor,NULL);
}





void BreakSink(int s)
{
	
	if(MicroStackChain!=NULL)
	{
		ILibStopChain(MicroStackChain);
		MicroStackChain = NULL;
	}
	
	

}





void* ILibPoolThread(void *args)
{
	ILibThreadPool_AddThread(ILib_Pool);
	return(0);
}




int main(void) 
{
	struct DMS_BackEndInit backendinit;

	
	int x;
	

	struct sigaction setup_action;
    sigset_t block_mask;
	pthread_t t;
     
		
	

	
	MicroStackChain = ILibCreateChain();
	
	

	/* TODO: Each device must have a unique device identifier (UDN) */
	
		
	
	/* All evented state variables MUST be initialized before MediaServer_Start is called. */
	
	

	
	printf("Intel MicroStack 1.0 - Digital Media Server (DLNA 1.5),\r\n\r\n");

	
	ILib_Pool = ILibThreadPool_Create();
	for(x=0;x<3;++x)
	{
		
		pthread_create(&t,NULL,&ILibPoolThread,NULL);
	}
	
	MediaServer_GetConfiguration()->Manufacturer = "DD-WRT";
	MediaServer_GetConfiguration()->ManufacturerURL = "http://www.dd-wrt.com";
	MediaServer_GetConfiguration()->ModelName = "DD-WRT Media Server";
	MediaServer_GetConfiguration()->ModelDescription = "Router with media sharing using attached USB storage.";
	MediaServer_GetConfiguration()->ModelNumber = "DD-WRT";
	MediaServer_GetConfiguration()->ModelURL = "http://www.dd-wrt.com";
	 backendinit.Path = "/usr/local/share";
	DMSObject = DMS_Create(
		MicroStackChain,
		ILib_Pool,
		0,
		"DD-WRT Media Server",
		"a0c59c94-68a4-429b-81fc-bb38283eabed",
		"0000001",
		0,
		0,
		"",
		"http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_SM,http-get:*:audio/L16;rate=44100;channels=2:DLNA.ORG_PN=LPCM,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_NTSC,http-get:*:audio/mpeg:DLNA.ORG_PN=MP3,http-get:*:audio/3gpp:DLNA.ORG_PN=AMR_3GPP,http-get:*:audio/mp4:DLNA.ORG_PN=AAC_ISO_320,http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_BASE,http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMABASE,http-get:*:video/x-ms-asf:DLNA.ORG_PN=MPEG4_P2_ASF_SP_G726,http-get:*:image/png:DLNA.ORG_PN=PNG_LRG",
		&backendinit,
		NULL);




	

	
	
	

	
	ILib_Monitor = ILibCreateLifeTime(MicroStackChain);
	
	ILib_IPAddressLength = ILibGetLocalIPAddressList(&ILib_IPAddressList);
	ILibLifeTime_Add(ILib_Monitor,NULL,4,(void*)&ILib_IPAddressMonitor,NULL);
	
	



	sigemptyset (&block_mask);
    /* Block other terminal-generated signals while handler runs. */
    sigaddset (&block_mask, SIGINT);
    sigaddset (&block_mask, SIGQUIT);
    setup_action.sa_handler = BreakSink;
    setup_action.sa_mask = block_mask;
    setup_action.sa_flags = 0;
    sigaction (SIGINT, &setup_action, NULL);


	
	ILibStartChain(MicroStackChain);


	
	

	
	if(ILib_Pool!=NULL)
	{
		printf("Stopping Thread Pool...\r\n");
		ILibThreadPool_Destroy(ILib_Pool);
		printf("Thread Pool Destroyed...\r\n");
	}
	

	
	free(ILib_IPAddressList);
	
	
	return 0;
}

