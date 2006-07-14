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
 * $Workfile: MimeTypes.h
 *
 *
 *
 */

#ifndef MIMETYPES_H

#include "CdsMediaClass.h"

/*
 *	Provides forward declarations for methods in
 *	MimeTypes.c
 */

#define CLASS_ITEM				"object.item"
#define CLASS_AUDIO_ITEM		"object.item.audioItem"
#define CLASS_PLAYLIST_M3U		"object.container.playlistContainer"
#define CLASS_PLAYLIST_ASX		"object.container.playlistContainer"
#define CLASS_PLAYLIST_DIDLS	"object.item.playlistItem"
#define CLASS_VIDEO_ITEM		"object.item.videoItem"
#define CLASS_IMAGE_ITEM		"object.item.imageItem"

/* mandatory by DLNA */
#define EXTENSION_IMAGE_JPG			".jpg"
#define EXTENSION_IMAGE_JPEG		".jpeg"
#define MIME_TYPE_IMAGE_JPEG		"image/jpeg"
#define PROTINFO_IMAGE_JPEG			"http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_SM"

#define EXTENSION_AUDIO_LPCM		".pcm"
#define MIME_TYPE_AUDIO_LPCM		"audio/L16;rate=44100;channels=2"
#define PROTINFO_AUDIO_LPCM			"http-get:*:audio/L16;rate=44100;channels=2:DLNA.ORG_PN=LPCM"

#define EXTENSION_VIDEO_MPEG2		".mpg"
#define MIME_TYPE_VIDEO_MPEG2		"video/mpeg"
#define PROTINFO_VIDEO_MPEG2		"http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_NTSC"

#define EXTENSION_AUDIO_MPEG		".mp3"
#define MIME_TYPE_AUDIO_MPEG		"audio/mpeg"
#define PROTINFO_AUDIO_MPEG			"http-get:*:audio/mpeg:DLNA.ORG_PN=MP3"

#define EXTENSION_AUDIO_3GPP		".3gp"
#define MIME_TYPE_AUDIO_3GPP		"audio/3gpp"
#define PROTINFO_AUDIO_3GPP			"http-get:*:audio/3gpp:DLNA.ORG_PN=AMR_3GPP"

#define EXTENSION_VIDEO_AAC			".mp4"
#define MIME_TYPE_VIDEO_AAC			"video/mp4"
#define PROTINFO_VIDEO_AAC			"http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF15_AAC_520"

#define EXTENSION_AUDIO_AAC			".mp4"
#define MIME_TYPE_AUDIO_AAC			"audio/mp4"
#define PROTINFO_AUDIO_AAC			"http-get:*:audio/mp4:DLNA.ORG_PN=AAC_ISO_320"

/* optional*/
#define EXTENSION_VIDEO_WMV			".wmv"
#define MIME_TYPE_VIDEO_WMV			"video/x-ms-wmv"
#define PROTINFO_VIDEO_WMV			"http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_BASE"

#define EXTENSION_AUDIO_WMA			".wma"
#define MIME_TYPE_AUDIO_WMA			"audio/x-ms-wma"
#define PROTINFO_AUDIO_WMA			"http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMABASE"

#define EXTENSION_VIDEO_ASF			".asf"
#define MIME_TYPE_VIDEO_ASF			"video/x-ms-asf"
#define PROTINFO_VIDEO_ASF			"http-get:*:video/x-ms-asf:DLNA.ORG_PN=MPEG4_P2_ASF_SP_G726"

#define EXTENSION_IMAGE_PNG			".png"
#define MIME_TYPE_IMAGE_PNG			"image/png"
#define PROTINFO_IMAGE_PNG			"http-get:*:image/png:DLNA.ORG_PN=PNG_LRG"

#define EXTENSION_AUDIO_WAV			".wav"
#define MIME_TYPE_AUDIO_WAV			"audio/wav"
#define PROTINFO_AUDIO_WAV			"http-get:*:audio/wav:*"

#define EXTENSION_PLAYLIST_ASX		".asx"
#define MIME_TYPE_PLAYLIST_ASX		"audio/x-ms-asx"
#define PROTINFO_PLAYLIST_ASX		"http-get:*:audio/x-ms-asx:*"

#define EXTENSION_IMAGE_TIF			".tif"
#define MIME_TYPE_IMAGE_TIF			"image/tif"
#define PROTINFO_IMAGE_TIF			"http-get:*:image/tif:*"

#define EXTENSION_IMAGE_GIF			".gif"
#define MIME_TYPE_IMAGE_GIF			"image/gif"
#define PROTINFO_IMAGE_GIF			"http-get:*:image/gif:*"

#define EXTENSION_IMAGE_BMP			".bmp"
#define MIME_TYPE_IMAGE_BMP			"image/bmp"
#define PROTINFO_IMAGE_BMP			"http-get:*:image/bmp:*"

#define EXTENSION_TXT				".txt"
#define MIME_TYPE_TXT				"text/plain"
#define PROTINFO_TXT				"http-get:*:text/plain:*"

#define EXTENSION_DIDLS				".didls"
#define MIME_TYPE_DIDLS				"text/xml"
#define PROTINFO_DIDLS				"http-get:*:text/xml:DLNA.ORG_PN=DIDL_S"

#define MIMETYPE_OCTETSTREAM		"application/octet-stream"
#define PROTINFO_OCTETSTREAM		"http-get:*:application/octet-stream:*"

/* mandatory by DLNA */
#define	DLNAPROFILE_JPEG_SM						"JPEG_SM"
#define	DLNAPROFILE_JPEG_MED					"JPEG_MED"
#define	DLNAPROFILE_JPEG_LRG					"JPEG_LRG"
#define	DLNAPROFILE_LPCM						"LPCM"
#define	DLNAPROFILE_MPEG_PS_NTSC				"MPEG_PS_NTSC"
#define	DLNAPROFILE_MP3							"MP3"
#define DLNAPROFILE_AMR_3GPP					"AMR_3GPP"
#define DLNAPROFILE_AVC_MP4_BL_CIF15_AAC_520	"AVC_MP4_BL_CIF15_AAC_520"
#define DLNAPROFILE_AAC_ISO_320					"AAC_ISO_320"

/* optional */
#define	DLNAPROFILE_WMVMED_BASE					"WMVMED_BASE"
#define	DLNAPROFILE_WMABASE						"WMABASE"
#define	DLNAPROFILE_MPEG4_P2_ASF_SP_G726		"MPEG4_P2_ASF_SP_G726"
#define	DLNAPROFILE_PNG_LRG						"PNG_LRG"
#define DLNAPROFILE_DIDL_S						"DIDL_S"


// user must free
char* FilePathToFileExtension(char* file_path, int wide);

/*
 *	FileExtensionToDlnaProfile()
 *
 *	Returns a DLNA.ORG_PN field.
 */
const char* FileExtensionToDlnaProfile(const char* extension, int wide);

/*
 *	GetMimeType()
 *		extension				: the file extension, including the dot '.' char
 *
 *	Returns the mime-type of a file with the given file extension.
 *	The method returns static values.
 *	Returns NULL if mapping cannot be determined.
 *	DO NOT CALL FREE ON THE RETURNED VALUE.
 */
const char* FileExtensionToMimeType (const char* extension, int wide);

/*
 *	MimeTypeToFileExtension()
 *		mime_type				: the mime-type
 *
 *	Returns the file extension of a file with the given mime type.
 *	The method returns static values.
 *	Returns NULL if mapping cannot be determined.
 *	DO NOT CALL FREE ON THE RETURNED VALUE.
 */
//const char* MimeTypeToFileExtension (const char* mime_type);

/*
 *	FileExtensionToClassCode()
 *
 *	Returns a media class value, as described by CdsMediaClass.h
 */
unsigned int FileExtensionToClassCode (const char* extension, int wide);
#endif
