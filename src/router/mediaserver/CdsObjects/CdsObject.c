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
 * $Workfile: CdsObject.c
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
	#include<windows.h>
#else
	#include <stdlib.h>
#endif

#include <string.h>
#include "CdsObject.h"

#ifndef _WIN32_WCE
	#include <assert.h>
	#define ASSERT(x) assert(x)
#endif

struct CdsObject* CDS_AllocateObject()
{
	struct CdsObject *cdsObj = (struct CdsObject *) malloc (sizeof(struct CdsObject));
	memset(cdsObj, 0, sizeof(struct CdsObject));
	cdsObj->CpInfo.Reserved.ServiceObject = NULL;
	sem_init(&(cdsObj->CpInfo.Reserved.ReservedLock), 0, 1);

	return cdsObj;
}

struct CdsResource* CDS_AllocateResource()
{
	struct CdsResource *res = (struct CdsResource *) malloc (sizeof(struct CdsResource));
	res->Allocated = 0;
	res->Flags = 0;
	res->Next = NULL;
	res->Value = res->Protection = res->ProtocolInfo = res->ImportUri = res->IfoFileUri = res->ImportIfoFileUri = NULL;
	res->ResumeUpload = 0;
	res->Size = res->ColorDepth = res->Bitrate = res->Duration = res->ResolutionX = res->ResolutionY = res->BitsPerSample = res->SampleFrequency = res->NrAudioChannels = res->UploadedSize = res->TrackTotal = -1;
	return res;
}

void _CDS_Deallocate_MultipleStrings(unsigned int deallocate_flag, int num_strings, char **strings)
{
	int i;

	if (
		(deallocate_flag) && 
		(num_strings > 0) &&
		(strings != NULL)
		)
	{
		for (i=0; i < num_strings; i++)
		{
			/* deallocate each album string */
			if (strings[i] != NULL)
			{
				free(strings[i]);
			}
		}
		/* deallocate the pointers for each of the strings */
		free(strings);
	}
}

void _CDS_Deallocate_Item(struct CdsObject *cds_obj)
{
	if (
		(cds_obj->DeallocateThese & CDS_ALLOC_RefID) &&
		(cds_obj->TypeObject.Item.RefID != NULL)
		)
	{
		free (cds_obj->TypeObject.Item.RefID);
	}
}

void _CDS_Deallocate_AudioItem(struct CdsObject *cds_obj)
{
	/* deallocate all of the albums */
	_CDS_Deallocate_MultipleStrings(
		cds_obj->DeallocateThese & CDS_ALLOC_Album, 
		cds_obj->TypeMajor.AudioItem.NumAlbums,
		cds_obj->TypeMajor.AudioItem.Albums);

	/* deallocate all of the genres */
	_CDS_Deallocate_MultipleStrings(
		cds_obj->DeallocateThese & CDS_ALLOC_Genre, 
		cds_obj->TypeMajor.AudioItem.NumGenres,
		cds_obj->TypeMajor.AudioItem.Genres);
}

void _CDS_Deallocate_VideoItem(struct CdsObject *cds_obj)
{
	/* deallocate all of the albums */
	_CDS_Deallocate_MultipleStrings(
		cds_obj->DeallocateThese & CDS_ALLOC_Genre, 
		cds_obj->TypeMajor.VideoItem.NumGenres,
		cds_obj->TypeMajor.VideoItem.Genres);
}

void _CDS_Deallocate_ImageItem(struct CdsObject *cds_obj)
{
	/* nothing to do for now */
}

void _CDS_Deallocate_MusicAlbum(struct CdsObject *cds_obj)
{
	/* deallocate all of the genres */
	_CDS_Deallocate_MultipleStrings(
		cds_obj->DeallocateThese & CDS_ALLOC_Genre, 
		cds_obj->TypeMinor1.MusicAlbum.NumGenres,
		cds_obj->TypeMinor1.MusicAlbum.Genres);
}

void _CDS_Deallocate_Photo(struct CdsObject *cds_obj)
{
	/* deallocate all of the genres */
	_CDS_Deallocate_MultipleStrings(
		cds_obj->DeallocateThese & CDS_ALLOC_Album, 
		cds_obj->TypeMinor1.Photo.NumAlbums,
		cds_obj->TypeMinor1.Photo.Albums);
}

void _CDS_Deallocate_AudioBroadcast(struct CdsObject *cds_obj)
{
	/* deallocate all of the genres */
	if (
		(cds_obj->DeallocateThese & CDS_ALLOC_ChannelName) &&
		(cds_obj->TypeMinor1.AudioBroadcast.ChannelName != NULL)
		)
	{
		free (cds_obj->TypeMinor1.AudioBroadcast.ChannelName);
	}
}

void _CDS_Deallocate_VideoBroadcast(struct CdsObject *cds_obj)
{
	/* deallocate all of the genres */
	if (
		(cds_obj->DeallocateThese & CDS_ALLOC_ChannelName) &&
		(cds_obj->TypeMinor1.VideoBroadcast.ChannelName != NULL)
		)
	{
		free (cds_obj->TypeMinor1.VideoBroadcast.ChannelName);
	}
}

void _CDS_Deallocate_CreateClasses(struct CdsCreateClass *classList)
{
	struct CdsCreateClass *createClass = classList, *next;

	while (createClass != NULL)
	{
		next = createClass->Next;
		free (createClass);
		createClass = next;
	}
}

void _CDS_Deallocate_SearchClasses(struct CdsSearchClass *classList)
{
	struct CdsSearchClass *searchClass = classList, *next;

	while (searchClass != NULL)
	{
		next = searchClass->Next;
		free (searchClass);
		searchClass = next;
	}
}

void CDS_DestroyObject(struct CdsObject *cds_obj)
{
	struct CdsObject *cdsobj = cds_obj;

	if(cdsobj != NULL)
	{
		/* Warning! This method is deprecated(),
		 * Do not call this method directly to destroy a created CdsObject using CDS_AllocateObject().
		 * Use \ref CDS_ObjRef_Release() instead.
		 */

		/* The ASSERT statement prevents you from calling DestroyObject directly on a newly created object. */
		ASSERT(cdsobj->CpInfo.Reserved.ServiceObject==NULL && cdsobj->CpInfo.Reserved.ReservedRefCount < 0);

		if ((cdsobj->DeallocateThese & CDS_ALLOC_Creator)	&& (cdsobj->Creator != NULL))	{ free(cdsobj->Creator); }
		if ((cdsobj->DeallocateThese & CDS_ALLOC_ID)		&& (cdsobj->ID != NULL))		{ free(cdsobj->ID); }
		if ((cdsobj->DeallocateThese & CDS_ALLOC_ParentID)	&& (cdsobj->ParentID != NULL))	{ free(cdsobj->ParentID); }
		if ((cdsobj->DeallocateThese & CDS_ALLOC_Title)		&& (cdsobj->Title != NULL))		{ free(cdsobj->Title); }

		switch (cdsobj->MediaClass & CDS_CLASS_MASK_OBJECT_TYPE)
		{

			case CDS_CLASS_MASK_CONTAINER:
				_CDS_Deallocate_CreateClasses(cdsobj->TypeObject.Container.CreateClass);
				_CDS_Deallocate_SearchClasses(cdsobj->TypeObject.Container.SearchClass);
				break;
			case CDS_CLASS_MASK_ITEM:
				_CDS_Deallocate_Item(cdsobj);
				break;
		}

		switch (cdsobj->MediaClass & CDS_CLASS_MASK_MAJOR)
		{
		case CDS_CLASS_MASK_MAJOR_AUDIOITEM:
			_CDS_Deallocate_AudioItem(cdsobj);
			break;
		case CDS_CLASS_MASK_MAJOR_IMAGEITEM:
			_CDS_Deallocate_ImageItem(cdsobj);
			break;
		case CDS_CLASS_MASK_MAJOR_VIDEOITEM:
			_CDS_Deallocate_VideoItem(cdsobj);
			break;

			/*	TODO: If you add more structs to union MajorTypes, 
				then add appropriate deallocator code here.

		case CDS_CLASS_MASK_MAJOR_ALBUM:
		case CDS_CLASS_MASK_MAJOR_GENRE:
		case CDS_CLASS_MASK_MAJOR_PERSON:
		case CDS_CLASS_MASK_MAJOR_PLAYLISTCONTAINER:
		case CDS_CLASS_MASK_MAJOR_STORAGEFOLDER:
		case CDS_CLASS_MASK_MAJOR_STORAGESYSTEM:
		case CDS_CLASS_MASK_MAJOR_STORAGEVOLUME:
		case CDS_CLASS_MASK_MAJOR_PLAYLISTITEM:
		case CDS_CLASS_MASK_MAJOR_TEXTITEM:
			break;
			*/
		}

		switch (cdsobj->MediaClass & CDS_CLASS_MASK_MINOR1)
		{
		case CDS_CLASS_MASK_MINOR1_MUSICALBUM:
			_CDS_Deallocate_MusicAlbum(cdsobj);
			break;
		case CDS_CLASS_MASK_MINOR1_PHOTO:
			_CDS_Deallocate_Photo(cdsobj);
			break;
		case CDS_CLASS_MASK_MINOR1_AUDIOBROADCAST:
			_CDS_Deallocate_AudioBroadcast(cdsobj);
			break;
		case CDS_CLASS_MASK_MINOR1_VIDEOBROADCAST:
			_CDS_Deallocate_VideoBroadcast(cdsobj);
			break;

			/*	TODO: If you add more structs to union Minor1Types, 
				then add appropriate deallocator code here.

		case CDS_CLASS_MASK_MINOR1_PHOTOALBUM:
		case CDS_CLASS_MASK_MINOR1_MOVIEGENRE:
		case CDS_CLASS_MASK_MINOR1_MUSICGENRE:
		case CDS_CLASS_MASK_MINOR1_MUSICARTIST:
		case CDS_CLASS_MASK_MINOR1_AUDIOBOOK:
		case CDS_CLASS_MASK_MINOR1_MUSICTRACK:
		case CDS_CLASS_MASK_MINOR1_MOVIE:
		case CDS_CLASS_MASK_MINOR1_MUSICVIDEOCLIP:
			break;
			*/
		}

		CDS_DestroyResources(cdsobj->Res);
	
		sem_destroy(&cdsobj->CpInfo.Reserved.ReservedLock);

		if(cdsobj->Source!=NULL)
		{
			free(cdsobj->Source);
		}
		free (cdsobj);
		cdsobj = NULL;
	}
}

void CDS_DestroyResources(struct CdsResource *resList)
{
	struct CdsResource *res = resList, *next;

	while (res != NULL)
	{
		next = res->Next;
		if ((res->Allocated & CDS_RES_ALLOC_Value) && (res->Value != NULL)) 
		{
			free (res->Value); 
		}
		
		if ((res->Allocated & CDS_RES_ALLOC_ProtocolInfo) && (res->ProtocolInfo != NULL)) 
		{
			free (res->ProtocolInfo); 
		}
		
		if ((res->Allocated & CDS_RES_ALLOC_Protection) && (res->Protection != NULL)) 
		{
			free (res->Protection); 
		}

		if ((res->Allocated & CDS_RES_ALLOC_IfoFileUri) && (res->IfoFileUri != NULL))
		{
			free (res->IfoFileUri);
		}

		if ((res->Allocated & CDS_RES_ALLOC_ImportIfoFileUri) && (res->ImportIfoFileUri != NULL))
		{
			free (res->ImportIfoFileUri);
		}

		if ((res->Allocated & CDS_RES_ALLOC_ImportUri) && (res->ImportUri != NULL))
		{
			free (res->ImportUri);
		}

		free (res);
		res = next;
	}
}
