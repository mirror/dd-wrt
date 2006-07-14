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
 * $Workfile: UTF8Utils.c
 *
 *
 *
 */

#if defined(WIN32) || defined(_WIN32_WCE)
#include <time.h>
#include <windows.h>
#endif

#include <stdio.h>
#include <string.h>
#include "UTF8Utils.h"
#include "ILibParsers.h"

 /*
 * Implements additional string functionality.
 */

char* EncodeToUTF8(const char* mbcsStr) 
{ 
#if defined(WIN32)
        wchar_t*  wideStr; 
        char*   utf8Str; 
        int   charLen; 

        charLen = MultiByteToWideChar(CP_ACP, 0, mbcsStr, -1, NULL, 0); 
        wideStr = (wchar_t*) malloc(sizeof(wchar_t)*charLen); 
        MultiByteToWideChar(CP_ACP, 0, mbcsStr, -1, wideStr, charLen); 

        charLen = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, NULL, 0, NULL, NULL); 

		utf8Str = (char*) malloc(charLen);

        WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, utf8Str, charLen, NULL, NULL);

        free(wideStr); 
        return utf8Str;
#else
	return(ILibString_Copy(mbcsStr, (int)strlen(mbcsStr)));
#endif
} 

char* DecodeFromUTF8(const char* utf8Str) 
{ 
#if defined(WIN32)
        wchar_t*  wideStr; 
        char*   mbcsStr; 
        int   charLen; 

        charLen = MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, NULL, 0); 
        wideStr = (wchar_t*) malloc(sizeof(wchar_t)*charLen); 
        MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, wideStr, charLen); 

        charLen = WideCharToMultiByte(CP_ACP, 0, wideStr, -1, NULL, 0, NULL, NULL); 

		mbcsStr = (char*) malloc(charLen);

        WideCharToMultiByte(CP_ACP, 0, wideStr, -1, mbcsStr, charLen, NULL, NULL);

        free(wideStr); 
        return mbcsStr;
#else
	return(ILibString_Copy(utf8Str, (int)strlen(utf8Str)));
#endif
} 