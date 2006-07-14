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
 * $Workfile: CdsMediaClass.c
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

#ifdef WIN32
#include <windows.h>
#endif

#include "CdsMediaClass.h"

/*
 *	The relative order of strings within these arrays must correspond to the MSCP_CLASS_MASK_xxx bitmask mappings.
 *	The first element must always be an empty string, to account for the fact that a sub-bitstring of the 
 *	mediaClass value may evaluate to 0.
 *
 *	All of these strings represent the normative set of media classes defined in UPnP AV.
 *
 *	It should be noted that none of these strings should EVER need to be change for XML escaping.
 */
const char* CDS_CLASS_OBJECT_TYPE[] = {"", "object.container", "object.item", };
const char* CDS_CLASS_MAJOR_TYPE[] = {"", "album", "genre", "person", "playlistContainer", "storageFolder", "storageSystem", "storageVolume", "audioItem", "imageItem", "playlistItem", "textItem", "videoItem"};
const char* CDS_CLASS_MINOR1_TYPE[] = {"", "musicAlbum", "photoAlbum", "movieGenre", "musicGenre", "musicArtist", "audioBook", "audioBroadcast", "musicTrack", "photo", "movie", "musicVideoClip", "videoBroadcast"};
const char* CDS_CLASS_MINOR2_TYPE[] = {""};
