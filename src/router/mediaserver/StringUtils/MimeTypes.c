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
 * $Workfile: MimeTypes.c
 *
 *
 *
 */

#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#ifndef _WIN32_WCE
#include <crtdbg.h>
#endif
#else
#include <stdlib.h>
#endif

#if defined(WINSOCK1)
#include <winsock.h>
#elif defined(WINSOCK2)
#include <winsock2.h>
#endif

#include <string.h>
#include "ILibParsers.h"
#include "DLNAProtocolInfo.h"
#include "MimeTypes.h"
#include "UTF8Utils.h"

#ifndef UNDER_CE
#include <wchar.h>
#endif

#ifdef _POSIX
#define stricmp strcasecmp
#endif

#define _TMP_CHAR_BUFFER 30

#define WIN32_DIR_DELIMITER			"\\"
#define WIN32_DIR_DELIMITER_LEN		2

char* FilePathToFileExtension(char* file_path, int wide)
{
	int slashPos;
	int dotPos;
	char* extension = NULL;
	int filePathLen = 0;

	if(file_path == NULL) return NULL;

	filePathLen = (int) strlen(file_path);

	slashPos = ILibString_LastIndexOf(file_path, filePathLen, WIN32_DIR_DELIMITER, WIN32_DIR_DELIMITER_LEN);
	dotPos = ILibString_LastIndexOf(file_path, filePathLen, ".", 1);


	if ((dotPos > 0) && (dotPos > slashPos))
	{
		// this is a file
		int extensionLen = (int) strlen(file_path) - dotPos;
		extension = (char*) malloc(extensionLen + 1);
		memcpy(extension, file_path + dotPos, extensionLen + 1);
	
		return extension;
	}

	// this is a folder
	return NULL;
	
}

const char* FileExtensionToDlnaProfile (const char* extension, int wide)
{
	char tempExtension[_TMP_CHAR_BUFFER];
	char *retVal = NULL;
	
	if (extension[0] != '.')
	{
		tempExtension[0] = '.';
		strcpy(tempExtension+1, extension);
		extension = tempExtension;
	}

	if (stricmp(extension, EXTENSION_IMAGE_JPG) == 0)
	{
		retVal = DLNAPROFILE_JPEG_SM;
	}
	else if (stricmp(extension, EXTENSION_IMAGE_JPEG) == 0)
	{
		retVal = DLNAPROFILE_JPEG_SM;
	}	
	else if (stricmp(extension, EXTENSION_AUDIO_LPCM) == 0)
	{
		retVal = DLNAPROFILE_LPCM;
	}
	else if (stricmp(extension, EXTENSION_VIDEO_MPEG2) == 0)
	{
		retVal = DLNAPROFILE_MPEG_PS_NTSC;
	}
	else if (stricmp(extension, EXTENSION_AUDIO_MPEG) == 0)
	{
		retVal = DLNAPROFILE_MP3;
	}
	else if (stricmp(extension, EXTENSION_AUDIO_3GPP) == 0)
	{
		retVal = DLNAPROFILE_AMR_3GPP;
	}
	else if (stricmp(extension, EXTENSION_AUDIO_AAC) == 0)
	{
		retVal = DLNAPROFILE_AVC_MP4_BL_CIF15_AAC_520;
	}
	else if (stricmp(extension, EXTENSION_AUDIO_AAC) == 0)
	{
		retVal = DLNAPROFILE_AAC_ISO_320;
	}
	else if (stricmp(extension, EXTENSION_VIDEO_WMV) == 0)
	{
		retVal = DLNAPROFILE_WMVMED_BASE;
	}
	else if (stricmp(extension, EXTENSION_AUDIO_WMA) == 0)
	{
		retVal = DLNAPROFILE_WMABASE;
	}
	else if (stricmp(extension, EXTENSION_VIDEO_ASF) == 0)
	{
		retVal = DLNAPROFILE_MPEG4_P2_ASF_SP_G726;
	}
	else if (stricmp(extension, EXTENSION_IMAGE_PNG) == 0)
	{
		retVal = DLNAPROFILE_PNG_LRG;
	}	
	else if (stricmp(extension, EXTENSION_DIDLS) == 0)
	{
		retVal = DLNAPROFILE_DIDL_S;
	}	
	else
	{
		retVal = "";
	}

	return retVal;
}

const char* FileExtensionToMimeType (const char* extension, int wide)
{
	char tempExtension[_TMP_CHAR_BUFFER];
	char *retVal = NULL;

	if(extension == NULL)
	{
		return NULL;
	}
	
	if (extension[0] != '.')
	{
		tempExtension[0] = '.';
		strcpy(tempExtension+1, extension);
		extension = tempExtension;
	}

	if (stricmp(extension, EXTENSION_IMAGE_JPEG) == 0)
	{
		retVal = MIME_TYPE_IMAGE_JPEG;
	}	
	else if (stricmp(extension, EXTENSION_IMAGE_JPG) == 0)
	{
		retVal = MIME_TYPE_IMAGE_JPEG;
	}	
	else if (stricmp(extension, EXTENSION_AUDIO_LPCM) == 0)
	{
		retVal = MIME_TYPE_AUDIO_LPCM;
	}
	else if (stricmp(extension, EXTENSION_VIDEO_MPEG2) == 0)
	{
		retVal = MIME_TYPE_VIDEO_MPEG2;
	}
	else if (stricmp(extension, EXTENSION_AUDIO_MPEG) == 0)
	{
		retVal = MIME_TYPE_AUDIO_MPEG;
	}
	else if (stricmp(extension, EXTENSION_AUDIO_3GPP) == 0)
	{
		retVal = MIME_TYPE_AUDIO_3GPP;
	}
	else if (stricmp(extension, EXTENSION_AUDIO_AAC) == 0)
	{
		retVal = MIME_TYPE_AUDIO_AAC;
	}
	else if (stricmp(extension, EXTENSION_IMAGE_PNG) == 0)
	{
		retVal = MIME_TYPE_IMAGE_PNG;
	}	
	else if (stricmp(extension, EXTENSION_VIDEO_WMV) == 0)
	{
		retVal = MIME_TYPE_VIDEO_WMV;
	}
	else if (stricmp(extension, EXTENSION_AUDIO_WMA) == 0)
	{
		retVal = MIME_TYPE_AUDIO_WMA;
	}
	else if (stricmp(extension, EXTENSION_VIDEO_ASF) == 0)
	{
		retVal = MIME_TYPE_VIDEO_ASF;
	}
	else if (stricmp(extension, EXTENSION_AUDIO_WAV) == 0)
	{
		retVal = MIME_TYPE_AUDIO_WAV;
	}
	else if (stricmp(extension, EXTENSION_PLAYLIST_ASX) == 0)
	{
		retVal = MIME_TYPE_PLAYLIST_ASX;
	}
	else if (stricmp(extension, EXTENSION_VIDEO_ASF) == 0)
	{
		retVal = MIME_TYPE_VIDEO_ASF;
	}
	else if (stricmp(extension, EXTENSION_IMAGE_TIF) == 0)
	{
		retVal = MIME_TYPE_IMAGE_TIF;
	}	
	else if (stricmp(extension, EXTENSION_IMAGE_GIF) == 0)
	{
		retVal = MIME_TYPE_IMAGE_GIF;
	}	
	else if (stricmp(extension, EXTENSION_IMAGE_BMP) == 0)
	{
		retVal = MIME_TYPE_IMAGE_BMP;
	}	
	else if (stricmp(extension, EXTENSION_TXT) == 0)
	{
		retVal = MIME_TYPE_TXT;
	}
	else if (stricmp(extension, EXTENSION_DIDLS) == 0)
	{
		retVal = MIME_TYPE_DIDLS;
	}
	else
	{
		retVal = MIMETYPE_OCTETSTREAM;
	}

	return retVal;
}

/*
const char* MimeTypeToFileExtension (const char* mime_type)
{
	if (stricmp(mime_type, MIME_TYPE_AUDIO_MPEG) == 0)
	{
		return EXTENSION_AUDIO_MPEG;
	}
	else if (stricmp(mime_type, MIME_TYPE_AUDIO_WMA) == 0)
	{
		return EXTENSION_AUDIO_WMA;
	}
	else if (stricmp(mime_type, MIME_TYPE_AUDIO_WAV) == 0)
	{
		return EXTENSION_AUDIO_WAV;
	}
	else if (stricmp(mime_type, MIME_TYPE_AUDIO_LPCM) == 0)
	{
		return EXTENSION_AUDIO_LPCM;
	}
	else if (stricmp(mime_type, MIME_TYPE_AUDIO_OGG) == 0)
	{
		return EXTENSION_AUDIO_OGG;
	}
	else if (stricmp(mime_type, MIME_TYPE_PLAYLIST_ASX) == 0)
	{
		return EXTENSION_PLAYLIST_ASX;
	}
	else if (stricmp(mime_type, MIME_TYPE_VIDEO_ASF) == 0)
	{
		return EXTENSION_VIDEO_ASF;
	}
	else if (stricmp(mime_type, MIME_TYPE_VIDEO_WMV) == 0)
	{
		return EXTENSION_VIDEO_WMV;
	}
	else if (stricmp(mime_type, MIME_TYPE_VIDEO_MPEG2) == 0)
	{
		return EXTENSION_VIDEO_MPEG2;
	}
	else if (stricmp(mime_type, MIME_TYPE_VIDEO_MOV) == 0)
	{
		return EXTENSION_VIDEO_MOV;
	}
	else if (stricmp(mime_type, MIME_TYPE_IMAGE_JPEG) == 0)
	{
		return EXTENSION_IMAGE_JPG;
	}
	else if (stricmp(mime_type, MIME_TYPE_IMAGE_PNG) == 0)
	{
		return EXTENSION_IMAGE_PNG;
	}
	else if (stricmp(mime_type, MIME_TYPE_IMAGE_GIF) == 0)
	{
		return EXTENSION_IMAGE_GIF;
	}
	else if (stricmp(mime_type, MIME_TYPE_IMAGE_TIF) == 0)
	{
		return EXTENSION_IMAGE_TIF;
	}
	else if (stricmp(mime_type, MIME_TYPE_IMAGE_JPEG) == 0)
	{
		return EXTENSION_IMAGE_JPEG;
	}
	else if (stricmp(mime_type, MIME_TYPE_IMAGE_BMP) == 0)
	{
		return EXTENSION_IMAGE_BMP;
	}
	else if (stricmp(mime_type, MIME_TYPE_PLAYLIST_M3U) == 0)
	{
		return EXTENSION_PLAYLIST_M3U;
	}

	return ".bin";
}
*/

unsigned int FileExtensionToClassCode (const char* extension, int wide)
{
	char tempExtension[_TMP_CHAR_BUFFER];
	unsigned int retVal;

	if (extension[0] != '.')
	{
		tempExtension[0] = '.';
		strcpy(tempExtension+1, extension);
		extension = tempExtension;
	}

	if (stricmp(extension, EXTENSION_IMAGE_JPG) == 0)
	{
		retVal = CDS_MEDIACLASS_IMAGEITEM;
	}
	else if (stricmp(extension, EXTENSION_IMAGE_JPEG) == 0)
	{
		retVal = CDS_MEDIACLASS_IMAGEITEM;
	}
	else if (stricmp(extension, EXTENSION_AUDIO_LPCM) == 0)
	{
		retVal = CDS_MEDIACLASS_AUDIOITEM;
	}
	else if (stricmp(extension, EXTENSION_VIDEO_MPEG2) == 0)
	{
		retVal = CDS_MEDIACLASS_VIDEOITEM;
	}
	else if (stricmp(extension, EXTENSION_AUDIO_MPEG) == 0)
	{
		retVal = CDS_MEDIACLASS_AUDIOITEM;
	}
	else if (stricmp(extension, EXTENSION_AUDIO_3GPP) == 0)
	{
		retVal = CDS_MEDIACLASS_AUDIOITEM;
	}
	else if (stricmp(extension, EXTENSION_AUDIO_AAC) == 0)
	{
		retVal = CDS_MEDIACLASS_AUDIOITEM;
	}
	else if (stricmp(extension, EXTENSION_VIDEO_WMV) == 0)
	{
		retVal = CDS_MEDIACLASS_VIDEOITEM;
	}
	else if (stricmp(extension, EXTENSION_AUDIO_WMA) == 0)
	{
		retVal = CDS_MEDIACLASS_AUDIOITEM;
	}
	else if (stricmp(extension, EXTENSION_VIDEO_ASF) == 0)
	{
		retVal = CDS_MEDIACLASS_VIDEOITEM;
	}
	else if (stricmp(extension, EXTENSION_IMAGE_PNG) == 0)
	{
		retVal = CDS_MEDIACLASS_IMAGEITEM;
	}	
	else if (stricmp(extension, EXTENSION_AUDIO_WAV) == 0)
	{
		retVal = CDS_MEDIACLASS_AUDIOITEM;
	}
	else if (stricmp(extension, EXTENSION_PLAYLIST_ASX) == 0)
	{
		retVal = CDS_MEDIACLASS_PLAYLISTCONTAINER;
	}
	else if (stricmp(extension, EXTENSION_IMAGE_TIF) == 0)
	{
		retVal = CDS_MEDIACLASS_IMAGEITEM;
	}	
	else if (stricmp(extension, EXTENSION_IMAGE_GIF) == 0)
	{
		retVal = CDS_MEDIACLASS_IMAGEITEM;
	}	
	else if (stricmp(extension, EXTENSION_IMAGE_BMP) == 0)
	{
		retVal = CDS_MEDIACLASS_IMAGEITEM;
	}
	else if (stricmp(extension, EXTENSION_TXT) == 0)
	{
		retVal = CDS_MEDIACLASS_ITEM;
	}
	else if (stricmp(extension, EXTENSION_DIDLS) == 0)
	{
		retVal = CDS_MEDIACLASS_PLAYLISTITEM;
	}
	else
	{
		retVal = CDS_MEDIACLASS_ITEM;
	}

	return retVal;
}
