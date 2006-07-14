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
 * $Workfile: FileSystemEnumerator.c
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

#if defined(WINSOCK1)
#include <winsock.h>
#elif defined(WINSOCK2)
#include <winsock2.h>
#endif

#include "FileSystemEnumerator.h"
#include "CdsErrors.h"
#include "ILibParsers.h"			
#include "FileIoAbstraction.h"
#include "CdsObject.h"
#include "CdsDidlSerializer.h"
#include "UTF8Utils.h"
#include "MimeTypes.h"
#include "CdsStrings.h"
#include "DLNAProtocolInfo.h"
#include <stdio.h>



/* slash and backslash */
#define FSE_BACKSLASH_CHR '\\'
#define FSE_BACKSLASH_STR "\\"
#define FSE_FORSLASH_CHR '/'
#define FSE_FORSLASH_STR "/"

enum FSE_Flags
{
	FSE_FLAG_Done			= 0x00000001,
	FSE_FLAG_ShowDot		= 0x00000002
};

/*
 *	Must match values returned by ILibFileDir_GetFileDirType()
 */
enum FSE_ObjectType
{
	FSE_OT_NoExist = 0,
	FSE_OT_File = 1,
	FSE_OT_Directory = 2
};

struct FSE_QueryState
{
	unsigned int Flags;
	enum FSE_CdsObjectType CdsObjectType;
	enum FSE_ObjectType ObjectType;
	char *FilePath;
	int FilePathLen;

	/*
	 *	TODO: Memory optimization can be achieved
	 *	by using a char* instead of a large array.
	 */
	char ProcessPath[MAX_FILENAME_SIZE];
	char PlaySingleProcessPath[MAX_FILENAME_SIZE];
	unsigned int ProcessIndex;
	enum FSE_ObjectType ProcessObjectType;
	void *ProcessDirHandle;
	int ProcessFileSize;
	unsigned int ProcessUpdateID;
};

struct FSE_State
{
	/*
	 *	The PreSelect, PostSelect, and Destroy fields must
	 *	remain here as the object needs to be compatible with the
	 *	thread-chaining framework.
	 */
	void (*PreSelect)(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);
	void (*PostSelect)(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
	void (*Destroy)(void* object);

	char *RootPath;
	int RootPathLen;
	char *VirtualDirPath;
	int  VirtualDirPathLen;
	char *DirDelimiter;
	char DirDelimiterChr;
	char BadDirDelimiterChr;
	void *ImportUriTable;
};

struct FSE_PathInfo
{
	char *FullPath;
	int FullPathLen;

	char *Name;
	int NameLen;

	char *NameNoExt;
	int NameNoExtLen;

	char *Ext;
	int ExtLen;

	char *ParentPath;
	int ParentPathLen;

	char *RelativeParent;
	int RelativeParentLen;

	char *RelativeFullPath;
	int RelativeFullPathLen;
};

/* declarations */
int _FSE_ExposeEntry(struct FSE_PathInfo *pi, struct FSE_State *fse, struct FSE_QueryState *qs);
int _FSE_FindNextEntry(struct FSE_State *fse, /*INOUT*/struct FSE_QueryState *qs, /*INOUT*/struct FSE_PathInfo *pi);
void _FSE_ParsePathInfo(const char* fullpath, const struct FSE_State *fse, const struct FSE_QueryState *qs, /*INOUT*/ struct FSE_PathInfo *pi);
void* _FSE_HandleQuery_Search(struct FSE_State *fse, struct FSE_QueryState *qs, struct MSA_CdsQuery *cdsQuery, unsigned int filter, /*OUT*/ enum Enum_CdsErrors *cdsErrorCode);

/*
 *	Returns nonzero if the CDS should expose this directory entry
 */
int _FSE_ExposeEntry(struct FSE_PathInfo *pi, struct FSE_State *fse, struct FSE_QueryState *qs)
{
	int retVal = 0;
	char badpath[10];
	int badpathlen = 0;

	/* CUSTOMIZE: Put special exclusions */
	if ((qs->Flags & FSE_FLAG_ShowDot))
	{
		if (strcmp(pi->FullPath, fse->RootPath) != 0) return 1;
	}
	else if (ILibString_EndsWith(pi->FullPath, pi->FullPathLen, fse->RootPath, fse->RootPathLen) != 0)
	{
		return 0;
	}
	
	badpathlen = sprintf(badpath, "..%s", fse->DirDelimiter);
	if (ILibString_EndsWith(pi->FullPath, pi->FullPathLen, badpath, badpathlen) != 0) return 0;

	badpathlen = sprintf(badpath, "%s%s", fse->DirDelimiter, fse->DirDelimiter);
	if (memcmp(pi->FullPath, badpath, badpathlen) == 0) return 0;

	/*
	 *	TODO: Customize for file system entries that should appear
	 *	appear in the CDS.
	 */

	/*
	 * NOTE: if the Operating System does not have the correct code-pages installed for the
	 * particular UNICODE fonts, then the file names will not be displayed correctly, therefore
	 * we are not going to expose it through CDS.  We are adding this check, as the file
	 * will not be found, although the processPath is correct.
	 */

	if (qs->ProcessObjectType == FSE_OT_NoExist) return 0;

	if (
		(qs->ProcessObjectType == FSE_OT_Directory) ||
		(strcmpi(pi->Ext,EXTENSION_AUDIO_MPEG)==0) ||
		(strcmpi(pi->Ext,EXTENSION_AUDIO_WMA)==0) ||
		(strcmpi(pi->Ext,EXTENSION_AUDIO_WAV)==0) ||
		(strcmpi(pi->Ext,EXTENSION_AUDIO_LPCM)==0) ||
		(strcmpi(pi->Ext,EXTENSION_VIDEO_ASF)==0) ||
		(strcmpi(pi->Ext,EXTENSION_VIDEO_WMV)==0) ||
		(strcmpi(pi->Ext,EXTENSION_VIDEO_MPEG2)==0) ||
		(strcmpi(pi->Ext,EXTENSION_IMAGE_PNG)==0) ||
		(strcmpi(pi->Ext,EXTENSION_IMAGE_TIF)==0) ||
		(strcmpi(pi->Ext,EXTENSION_IMAGE_GIF)==0) ||
		(strcmpi(pi->Ext,EXTENSION_IMAGE_JPG)==0) ||
		(strcmpi(pi->Ext,EXTENSION_IMAGE_BMP)==0) ||
//		(strcmpi(pi->Ext,EXTENSION_TXT)==0) ||
		(strcmpi(pi->Ext,EXTENSION_UPLOAD_TEMP)==0) ||
		(strcmpi(pi->Ext,EXTENSION_DIDLS)==0)
		)
	{
		retVal = 1;
	}


	return retVal;
}

/*
 *	Updates 'qs' with the next file of interest.
 *	If nothing is left, qs->Flags will have FSE_FLAG_Done set.
 *	Returns 0 if nothing was found.
 */
int _FSE_FindNextEntry(struct FSE_State *fse, /*INOUT*/struct FSE_QueryState *qs, /*INOUT*/struct FSE_PathInfo *pi)
{
	int expose = 0;
	char fn[MAX_FILENAME_SIZE];

	if (qs->ProcessDirHandle != NULL)
	{
		/*
		 *	If we're executing this, we're definitely processing
		 *	BrowseDirectChildren. This means that qs->FilePath
		 *	will have the base directory of all returned child objects.
		 *	Therefore, we can set qs->ProcessPath to qs->FilePath,
		 *	and append with the filename. Furthermore, we can assume
		 *	that qs->FilePath will already have a trailing delimiter
		 *	because it was done in FSE_HandleQuery.
		 */

		while (ILibFileDir_GetDirNextFile(
					qs->ProcessDirHandle, 
					qs->FilePath, 
					fn,
					MAX_FILENAME_SIZE, 
					&(qs->ProcessFileSize)
					) != 0)
		{
			/*
			 *	A file was found. Let's see if it's something of interest.
			 */
			if ( !
				((strcmp(fn, ".") == 0) ||
				(strcmp(fn, "..") == 0))
				)
			{
				sprintf(qs->ProcessPath, "%s%s", qs->FilePath, fn);
#if defined (INCLUDE_FEATURE_PLAYSINGLE)
REPROCESS:
#endif
				qs->ProcessObjectType = ILibFileDir_GetType(qs->ProcessPath);
				if(
					(qs->ProcessObjectType==ILibFileDir_Type_FILE) &&
					(ILibString_EndsWith(qs->ProcessPath, (int) strlen(qs->ProcessPath), EXTENSION_UPLOAD_TEMP, EXTENSION_UPLOAD_TEMP_LEN)!=0)
				)
				{
					/* treat incomplete upload files as CDS items. */
					qs->CdsObjectType = FSE_CDS_TEMPOPRARY;
				}
				_FSE_ParsePathInfo(qs->ProcessPath, fse, qs, pi);

				if (_FSE_ExposeEntry(pi, fse, qs) != 0)
				{
					expose = 1;
				}

				if (expose != 0) break;
			}
		}
	}

	#if defined (INCLUDE_FEATURE_PLAYSINGLE)
	/* if playsingle simulation is enabled, and there is a playsingle item available
	 * repeat the process.
	 */
	if(qs->PlaySingleProcessPath[0] != '\0')
	{
		qs->CdsObjectType = FSE_CDS_PLAYSINGLE;
		/* replace the process path with the last valid CDS item process path. */
		strcpy(qs->ProcessPath, qs->PlaySingleProcessPath);
		qs->PlaySingleProcessPath[0] = '\0';
		goto REPROCESS;
	}
	#endif

	if (expose == 0)
	{
		/* no suitable entries were found, so mark it as being done */
		qs->Flags |= FSE_FLAG_Done;

		/*
		 *	free temp storage
		 */
		if (pi->Ext != NULL) { free (pi->Ext); pi->Ext = NULL; pi->ExtLen = 0; }
		if (pi->FullPath != NULL) { free (pi->FullPath); pi->FullPath = NULL; pi->FullPathLen = 0; }
		if (pi->Name != NULL) { free (pi->Name); pi->Name = NULL; pi->NameLen = 0; }
		if (pi->NameNoExt != NULL) { free (pi->NameNoExt); pi->NameNoExt = NULL; pi->NameNoExtLen = 0; }
		if (pi->ParentPath != NULL) { free (pi->ParentPath); pi->ParentPath = NULL; pi->ParentPathLen = 0; }
		if (pi->RelativeFullPath != NULL) { free (pi->RelativeFullPath); pi->RelativeFullPath = NULL; pi->RelativeFullPathLen = 0; } 
		if (pi->RelativeParent != NULL) { free (pi->RelativeParent); pi->RelativeParent = NULL; pi->RelativeParentLen = 0; }
	}

	return expose;
}

void _FSE_ParsePathInfo(const char* fullpath, const struct FSE_State *fse, const struct FSE_QueryState *qs, /*INOUT*/ struct FSE_PathInfo *pi)
{
	int len;
	int ddpos=0;
	int dotpos=0;

	/* free memory */
	if (pi->Ext != NULL) { free (pi->Ext); pi->Ext = NULL; pi->ExtLen = 0; }

	if (pi->FullPath != NULL) { free (pi->FullPath); pi->FullPath = NULL; pi->FullPathLen = 0; }
	if (pi->Name != NULL) { free (pi->Name); pi->Name = NULL; pi->NameLen = 0; }
	if (pi->NameNoExt != NULL) { free (pi->NameNoExt); pi->NameNoExt = NULL; pi->NameNoExtLen = 0; }
	if (pi->ParentPath != NULL) { free (pi->ParentPath); pi->ParentPath = NULL; pi->ParentPathLen = 0; }
	if (pi->RelativeFullPath != NULL) { free (pi->RelativeFullPath); pi->RelativeFullPath = NULL; pi->RelativeFullPathLen = 0; } 
	if (pi->RelativeParent != NULL) { free (pi->RelativeParent); pi->RelativeParent = NULL; pi->RelativeParentLen = 0; }

	len = (int) strlen(fullpath);
	pi->FullPath = (char*) malloc(len+2);
	memcpy(pi->FullPath, fullpath, len);
	pi->FullPath[len] = '\0';
	pi->FullPathLen = len;

	/*
	 *	If it's a directory, make sure it ends
	 *	with a trailing directory delimiter.
	 *	We do this because fse->RootPath will have
	 *	a trailing delimiter.
	 */
	if (
		(qs->ProcessObjectType == FSE_OT_Directory) &&
		(pi->FullPath[pi->FullPathLen-1] != fse->DirDelimiterChr)
		)
	{
		pi->FullPath[pi->FullPathLen] = fse->DirDelimiterChr;
		pi->FullPathLen++;
		pi->FullPath[pi->FullPathLen] = '\0';
	}

	/*
	 *	Get the relative full path.
	 */
	pi->RelativeFullPathLen = pi->FullPathLen - fse->RootPathLen;
	pi->RelativeFullPath = (char*) malloc (pi->RelativeFullPathLen + 1);
	memcpy(pi->RelativeFullPath, pi->FullPath + fse->RootPathLen, pi->RelativeFullPathLen);
	pi->RelativeFullPath[pi->RelativeFullPathLen] = '\0';

	/*
	 *	Find last directory delimiter.
	 *	If the fullpath is a directory, temporarily
	 *	terminate w/o a trailing delimiter.
	 */

	if (qs->ProcessObjectType == FSE_OT_Directory)
	{
		pi->FullPathLen--;
		pi->FullPath[pi->FullPathLen] = '\0';
	}
	ddpos = ILibString_LastIndexOf(pi->FullPath, pi->FullPathLen, fse->DirDelimiter, (int) strlen(fse->DirDelimiter));
	if (qs->ProcessObjectType == FSE_OT_Directory)
	{
		pi->FullPath[pi->FullPathLen] = fse->DirDelimiterChr;
		pi->FullPathLen++;
	}

	/*
	 *	Get the name of the directory/file.
	 *	If no directory delimiter was found,
	 *	there is no name... which indicates
	 *	we should treat it as the root.
	 */
	if (ddpos >= 0)
	{
		pi->NameLen = pi->FullPathLen - ddpos - 1;
		pi->Name = (char*) malloc(pi->NameLen+1);
		memcpy(pi->Name, pi->FullPath + ddpos + 1, pi->NameLen+1);
	}
	else
	{
		pi->NameLen = 0;
		pi->Name = (char*) malloc(1);
		pi->Name[0] = '\0';
	}


	/*
	 *	Chop a trailing dir delimiter.
	 */
	if (
		(pi->NameLen > 0) && 
		(pi->Name[pi->NameLen-1] == fse->DirDelimiterChr)
		)
		{
			pi->NameLen--;
			pi->Name[pi->NameLen] = '\0';
		}

		dotpos = ILibString_LastIndexOf(pi->Name, pi->NameLen, ".", 1);
	if (dotpos >= 0)
	{
		/*
		 *	Get the name w/o the extension and save extension.
		 */
		pi->NameNoExtLen = dotpos;
		pi->NameNoExt = (char*) malloc(pi->NameNoExtLen + 1);
		memcpy(pi->NameNoExt, pi->Name, pi->NameNoExtLen);
		pi->NameNoExt[pi->NameNoExtLen] = '\0';

		pi->ExtLen = pi->NameLen - pi->NameNoExtLen;
		pi->Ext = (char*) malloc (pi->ExtLen + 1);
		memcpy(pi->Ext, pi->Name + dotpos, pi->ExtLen+1);
	}
	else
	{
		/*
		 *	Name w/o extension is the same as name.
		 *	Extension is blank.
		 */
		pi->NameNoExt = (char*) malloc(pi->NameLen+1);
		memcpy(pi->NameNoExt, pi->Name, pi->NameLen+1);
		pi->NameNoExtLen = pi->NameLen;

		pi->ExtLen = 0;
		pi->Ext = (char*) malloc(pi->ExtLen+1);
		pi->Ext[0] = '\0';
	}

	/*
	 *	Get the parent directory name
	 */
	if (ddpos >= 0)
	{
		pi->ParentPathLen = ddpos+1;
		pi->ParentPath = (char*) malloc(pi->ParentPathLen+1);
		memcpy(pi->ParentPath, pi->FullPath, pi->ParentPathLen);
		pi->ParentPath[pi->ParentPathLen] = '\0';
	}
	else
	{
		pi->ParentPathLen = 0;
		pi->ParentPath = (char*) malloc(1);
		pi->ParentPath[0] = '\0';
	}

	/* 
	 *	Get the relative parent dir.
	 *	rootlen includes a trailinger dir delimiter.
	 */
	if ((pi->ParentPathLen > 0) && (pi->ParentPathLen >= fse->RootPathLen))
	{
		pi->RelativeParentLen = pi->ParentPathLen - fse->RootPathLen;
		pi->RelativeParent = (char*) malloc(pi->RelativeParentLen + 1);
		memcpy(pi->RelativeParent, pi->ParentPath + fse->RootPathLen, pi->RelativeParentLen);
		pi->RelativeParent[pi->RelativeParentLen] = '\0';
	}
	else
	{
		pi->RelativeParentLen = 0;
		pi->RelativeParent = (char*) malloc(1);
		pi->RelativeParent[0] = '\0';
	}

}

void* _FSE_HandleQuery_Search(struct FSE_State *fse, struct FSE_QueryState *qs, struct MSA_CdsQuery *cdsQuery, unsigned int filter, /*OUT*/ enum Enum_CdsErrors *cdsErrorCode)
{
	/*
	 *	All containers are marked as non-searchable. (e.g. @searchable="0").
	 *	CDS spec says to reutn zero results. 
	 *	Furthermore, there is no error code defined for this case.
	 */

	qs->Flags |= FSE_FLAG_Done;
	return qs;
}

void* FSE_HandleQuery(void *fseObj, struct MSA_CdsQuery *cdsQuery, unsigned int filter, /*OUT*/ enum Enum_CdsErrors *cdsErrorCode)
{
	int objIdLen;
	struct FSE_State *fse = (struct FSE_State*) fseObj;
	struct FSE_QueryState *qs = NULL;
	char ProcessPath[MAX_FILENAME_SIZE ];
	char* tempPtr;

	/*
	*	SOLUTION_REFERENCE#3.6.3.2b
	*/

	*cdsErrorCode = CdsError_None;

	qs = (struct FSE_QueryState*) malloc (sizeof(struct FSE_QueryState));
	memset(qs, 0, sizeof(struct FSE_QueryState));

	/*
	 *	Set qs->FilePath
	 */
	objIdLen = (int) strlen(cdsQuery->ObjectID);
	qs->FilePath = (char*) malloc(fse->RootPathLen + objIdLen + 10);
	if (objIdLen > 2)
	{
		switch (cdsQuery->QueryType)
		{
		case MSA_Query_BrowseMetadata:
			sprintf(qs->FilePath, "%s%s", fse->RootPath, cdsQuery->ObjectID+2);
			break;

		default:
			sprintf(qs->FilePath, "%s%s", fse->RootPath, cdsQuery->ObjectID+2);
			break;
		}
	}
	else
	{
		sprintf(qs->FilePath, "%s", fse->RootPath);
	}

	qs->FilePathLen = (int) strlen(qs->FilePath);
	tempPtr = qs->FilePath;
	qs->FilePath = ILibString_Replace(tempPtr, qs->FilePathLen, &fse->BadDirDelimiterChr, 1, &fse->DirDelimiterChr, 1);
	qs->FilePathLen = (int) strlen(qs->FilePath);
	free(tempPtr);

	/* check to see what type of file system entry this is */
	qs->ObjectType = ILibFileDir_GetType(qs->FilePath);

	switch(qs->ObjectType)
	{
	case FSE_OT_NoExist:

		/* Item not found, could be looking for incomplete CDS objects that didn't finish upload,
		 * these items are saved in temp files in a .tmp extension, try locating those and if not
		 * found, then return object id not exist error */
		tempPtr = malloc(qs->FilePathLen + EXTENSION_UPLOAD_TEMP_LEN + 1);
		sprintf(tempPtr, "%s%s", qs->FilePath, EXTENSION_UPLOAD_TEMP);
		qs->ObjectType = ILibFileDir_GetType(tempPtr);
		if(qs->ObjectType == FSE_OT_File)
		{
			/* temp file availble */
			strcpy(qs->ProcessPath, tempPtr);
			qs->ProcessObjectType = qs->ObjectType;
			qs->CdsObjectType = FSE_CDS_TEMPOPRARY;
			free(tempPtr);
			return qs;
			break;
		}
		else
		{
			/* file not exist or */
			*cdsErrorCode = CDS_EC_OBJECT_ID_NO_EXIST;
			free(tempPtr);
			break;
		}
	
	case FSE_OT_File:
		switch (cdsQuery->QueryType)
		{
		case MSA_Query_BrowseMetadata:
			qs->ProcessIndex = 0;
			qs->ProcessObjectType  = qs->ObjectType;
			strcpy(qs->ProcessPath, qs->FilePath);
			return qs;
			break;
		
		case MSA_Query_BrowseDirectChildren:
		case MSA_Query_Search:
			*cdsErrorCode = CDS_EC_NO_SUCH_CONTAINER;
			break;
		}
		break;

	case FSE_OT_Directory:
		/*
		 *	If it's a directory, ensure it has
		 *	a trailing delimiter.
		 */
		if (qs->FilePath[qs->FilePathLen-1] != fse->DirDelimiterChr)
		{
			qs->FilePath[qs->FilePathLen] = fse->DirDelimiterChr;
			qs->FilePathLen++;
			qs->FilePath[qs->FilePathLen] = '\0';
		}

		/*
		 *	If the query is on the rootpath,
		 *	allow the CDS to report "." entries.
		 */
		if (
			(cdsQuery->QueryType == MSA_Query_BrowseMetadata) &&
			(strcmp(qs->FilePath, fse->RootPath) == 0)
			)
		{
			qs->Flags |= FSE_FLAG_ShowDot;
		}

		switch (cdsQuery->QueryType)
		{
		case MSA_Query_BrowseMetadata:
			qs->ProcessIndex = 0;
			qs->ProcessObjectType  = qs->ObjectType;
			strcpy(qs->ProcessPath, qs->FilePath);
			return qs;
			break;

		case MSA_Query_BrowseDirectChildren:
			qs->ProcessIndex = 0;
			qs->ProcessDirHandle = ILibFileDir_GetDirFirstFile(qs->FilePath, ProcessPath, MAX_FILENAME_SIZE, &(qs->ProcessFileSize));
			
			if(qs->ProcessDirHandle == NULL)
			{
				sprintf(qs->ProcessPath, "%s", "");
			}
			else if ( !
				((strcmp(ProcessPath, ".") == 0) ||
				(strcmp(ProcessPath, "..") == 0))
				)
			{
				sprintf(qs->ProcessPath, "%s%s", qs->FilePath, ProcessPath);
				qs->ProcessObjectType = ILibFileDir_GetType(qs->ProcessPath);
				if(
					(qs->ProcessObjectType==ILibFileDir_Type_FILE) &&
					(ILibString_EndsWith(qs->ProcessPath, (int) strlen(qs->ProcessPath), EXTENSION_UPLOAD_TEMP, EXTENSION_UPLOAD_TEMP_LEN)!=0)
				)
				{
					/* treat incomplete upload files as CDS items. */
					qs->CdsObjectType = FSE_CDS_TEMPOPRARY;
				}
			}
			else
			{
				sprintf(qs->ProcessPath, "%s", ProcessPath);
				qs->ProcessObjectType = ILibFileDir_GetType(qs->ProcessPath);
			}


			return qs;
			break;

		case MSA_Query_Search:
			return _FSE_HandleQuery_Search(fse, qs, cdsQuery, filter, cdsErrorCode);
			break;
		}
		break;

	default:
		*cdsErrorCode = CDS_EC_ACTION_FAILED;
		break;
	}

	/* return NULL with the nonzero *cdsErrorCode */
	return NULL;
}

int FSE_HandleProcessObject(void *fseObj, struct MSA_CdsQuery *cdsQuery, void *onQueryObj, unsigned int filter, /*INOUT*/ struct CdsObject *cdsObj)
{
	struct FSE_State *fse = (struct FSE_State*) fseObj;
	struct FSE_QueryState *qs = (struct FSE_QueryState*) onQueryObj;
	struct FSE_PathInfo pi;
	struct FSE_ResourceMetadata* fseRes;
	int i;
	int expose = 0;
	int ignore = 0;
	char fn[MAX_FILENAME_SIZE];

	fseRes = (struct FSE_ResourceMetadata*) malloc(sizeof(struct FSE_ResourceMetadata));
	memset(fseRes, 0, sizeof(struct FSE_ResourceMetadata));

	memset(&pi, 0, sizeof(struct FSE_PathInfo));

	if (qs == NULL)
	{
		fprintf(stderr, "FSE_HandleProcessObject(): qs == NULL\r\n");
	}
	if (cdsObj == NULL)
	{
		fprintf(stderr, "FSE_HandleProcessObject(): cdsObj == NULL\r\n");
	}

	if ((qs->Flags & FSE_FLAG_Done) == 0)
	{
		/*
		 *	Do not process "." or ".."
		 */
		if (
			(strcmp(qs->ProcessPath, "")==0) ||
			(strcmp(qs->ProcessPath, ".")==0) ||
			(strcmp(qs->ProcessPath, "..")==0)
			)
		{
			ignore = 1;
		}

		/* get info about this path and determine if we're going to expose it */
		if (ignore == 0)
		{
			/*
			*	free temp storage
			*/
			if (pi.Ext != NULL) { free (pi.Ext); pi.Ext = NULL; pi.ExtLen = 0; }
			if (pi.FullPath != NULL) { free (pi.FullPath); pi.FullPath = NULL; pi.FullPathLen = 0; }
			if (pi.Name != NULL) { free (pi.Name); pi.Name = NULL; pi.NameLen = 0; }
			if (pi.NameNoExt != NULL) { free (pi.NameNoExt); pi.NameNoExt = NULL; pi.NameNoExtLen = 0; }
			if (pi.ParentPath != NULL) { free (pi.ParentPath); pi.ParentPath = NULL; pi.ParentPathLen = 0; }
			if (pi.RelativeFullPath != NULL) { free (pi.RelativeFullPath); pi.RelativeFullPath = NULL; pi.RelativeFullPathLen = 0; } 
			if (pi.RelativeParent != NULL) { free (pi.RelativeParent); pi.RelativeParent = NULL; pi.RelativeParentLen = 0; }


			memset(&pi, 0, sizeof(struct FSE_PathInfo));
			_FSE_ParsePathInfo(qs->ProcessPath, fse, qs, &pi);


			/* populate pi with data */
			expose = _FSE_ExposeEntry(&pi, fse, qs);
			if(cdsQuery->QueryType == MSA_Query_BrowseMetadata)
			{
				qs->Flags |= FSE_FLAG_Done;
			}
		}
		

		/*
		 * If this file is not supported, or it is a temp file where duplicated, we don't expose it,
		 * we have to find the next valid entry.
		 */
		if (ignore == 1 || expose == 0)
		{
			expose = _FSE_FindNextEntry(fse, qs, &pi);
		}

		/*
		*	SOLUTION_REFERENCE#3.6.3.2d
		*/


		if (expose != 0)
		{
			if(qs->CdsObjectType == FSE_CDS_PLAYSINGLE)
			{
				cdsObj->ID = (char*) malloc(pi.RelativeFullPathLen+CDS_STRING_PLAYSINGLE_ITEM_PREFIX_LEN+4);
			}
			else
			{
				cdsObj->ID = (char*) malloc(pi.RelativeFullPathLen+4);
			}
			cdsObj->ParentID = (char*) malloc(pi.RelativeParentLen+4);
			if (pi.RelativeFullPathLen > 0)
			{
				if(qs->CdsObjectType == FSE_CDS_PLAYSINGLE)
				{
					sprintf(cdsObj->ID, "%s0%s%s", CDS_STRING_PLAYSINGLE_ITEM_PREFIX, fse->DirDelimiter, pi.RelativeFullPath);
				}
				else
				{
					sprintf(cdsObj->ID, "0%s%s", fse->DirDelimiter, pi.RelativeFullPath);
				}
				if(ILibString_EndsWith(cdsObj->ID,(int)strlen(cdsObj->ID),EXTENSION_UPLOAD_TEMP,EXTENSION_UPLOAD_TEMP_LEN)!=0)
				{
					cdsObj->ID[(int)strlen(cdsObj->ID)-EXTENSION_UPLOAD_TEMP_LEN]=0;
				}

				if (pi.RelativeParentLen != 0)
				{
					sprintf(cdsObj->ParentID, "0%s%s", fse->DirDelimiter, pi.RelativeParent);
				}
				else
				{
					strcpy(cdsObj->ParentID, "0");
				}
			}
			else
			{
				sprintf(cdsObj->ID, "0");
				sprintf(cdsObj->ParentID, "-1");
			}

			if(qs->ProcessObjectType == FSE_OT_Directory && pi.NameLen > 0)
			{
				/* this is a directory, use the full directory name */
				cdsObj->Title = (char*) malloc(pi.NameLen+1);
				strcpy(cdsObj->Title, pi.Name);
			}
			else if (pi.NameNoExtLen > 0)
			{
				/* this is a file, use the name without extenstion for its title */
				cdsObj->Title = (char*) malloc(pi.NameNoExtLen+1);
				strcpy(cdsObj->Title, pi.NameNoExt);

				if(qs->CdsObjectType == FSE_CDS_TEMPOPRARY)
				{
					/* remove actual file extension in front of the tmp file extension */
					i = ILibString_LastIndexOf(cdsObj->Title, (int) strlen(cdsObj->Title),".",1);
					cdsObj->Title[i] = 0;				 
				}
			}
			else if (
				(pi.RelativeFullPathLen == 0) &&
				(pi.RelativeParentLen == pi.RelativeFullPathLen)
				)
			{
				/* this is the root directory */
				cdsObj->Title = (char*) malloc(5);
				strcpy(cdsObj->Title, "Root");
			}
			else
			{
				/* unknown item */
				cdsObj->Title = (char*) malloc(10);
				strcpy(cdsObj->Title, "<Unknown>");
			}

			cdsObj->Source = (char*)malloc(pi.FullPathLen+1);
			memcpy(cdsObj->Source,pi.FullPath,pi.FullPathLen);
			cdsObj->Source[pi.FullPathLen]=0;
			
			cdsObj->DeallocateThese =
				CDS_ALLOC_ID |
				CDS_ALLOC_ParentID |
				CDS_ALLOC_Title;

			/*
			*	Set the media class, the file system will consider the file type to be unknown if
			*   the file extension is something it isn't aware of, so if we are processing
			*   a temp file, then also need to find out its media class.
			*/
			if (qs->ProcessObjectType == FSE_OT_File)
			{
				if(strcmp(EXTENSION_UPLOAD_TEMP,pi.Ext)==0)
				{
					i = ILibString_LastIndexOf(pi.FullPath,pi.FullPathLen-EXTENSION_UPLOAD_TEMP_LEN,".",1);
					pi.FullPath[pi.FullPathLen-EXTENSION_UPLOAD_TEMP_LEN]=0;
					cdsObj->MediaClass = FileExtensionToClassCode(pi.FullPath+i, 0);
					pi.FullPath[pi.FullPathLen-EXTENSION_UPLOAD_TEMP_LEN]='.';
				}
				else
				{
					cdsObj->MediaClass = FileExtensionToClassCode(pi.Ext, 0);
				}
			}
			else if (qs->ProcessObjectType == FSE_OT_Directory)
			{
				cdsObj->MediaClass = CDS_MEDIACLASS_STORAGEFOLDER;
			}
			else
			{
				/* 
				 *	TODO: If you have other types of classes
				 *	you'll need to modify this section.
				 */
				cdsObj->MediaClass = CDS_MEDIACLASS_CONTAINER;
			}

			/*
			 * CUSTOMIZE: Additional meta data for ITEM
			 */
			if ((cdsObj->MediaClass & CDS_CLASS_MASK_OBJECT_TYPE) == CDS_CLASS_MASK_ITEM)
			{
				/*
				 *  All items supports OCM: detroy item
				 */
				cdsObj->DlnaManaged = CDS_DlnaManaged_DestroyItem;

				/* If this is image/video item, then get the last modified timestamp of the file
				 * and use it for the <dc:date> value
				 */
				switch (cdsObj->MediaClass & CDS_CLASS_MASK_MAJOR)
				{
					case CDS_CLASS_MASK_MAJOR_AUDIOITEM:
						cdsObj->TypeMajor.AudioItem.Date = (long) ILibFileDir_GetFileTimeStamp(cdsObj->Source);
						break;
					case CDS_CLASS_MASK_MAJOR_IMAGEITEM:
						cdsObj->TypeMajor.ImageItem.Date = (long) ILibFileDir_GetFileTimeStamp(cdsObj->Source);
						break;
					case CDS_CLASS_MASK_MAJOR_VIDEOITEM:
						cdsObj->TypeMajor.VideoItem.Date = (long) ILibFileDir_GetFileTimeStamp(cdsObj->Source);
						break;
				}

				#if defined (INCLUDE_FEATURE_PLAYSINGLE)
				/*
				*	SOLUTION_REFERENCE#3.6.3.11a
				*/

				/* If playsingle simulation is enabled, and if it is a audio/image/video item,
				 * then save the item's processpath, so that we'll use it to create a playsingle CdsObject
				 */
				if(
					(qs->CdsObjectType == FSE_CDS_NORMAL) && 
					(
					((cdsObj->MediaClass & CDS_CLASS_MASK_MAJOR) == CDS_CLASS_MASK_MAJOR_AUDIOITEM) ||
					((cdsObj->MediaClass & CDS_CLASS_MASK_MAJOR) == CDS_CLASS_MASK_MAJOR_IMAGEITEM) ||
					((cdsObj->MediaClass & CDS_CLASS_MASK_MAJOR) == CDS_CLASS_MASK_MAJOR_VIDEOITEM)
					)
				)
				{
					strcpy(qs->PlaySingleProcessPath, qs->ProcessPath);
				}
				#endif
			}
	
			/*
			 * CUSTOMIZE: Additional meta data for CONTAINER
			 */
			if ((cdsObj->MediaClass & CDS_CLASS_MASK_OBJECT_TYPE) == CDS_CLASS_MASK_CONTAINER)
			{
				/*
				 *  Containers are not searchable in this implementation.
				 *	CUSTOMIZE: Set this if container is searchable.
				 *
				 *	cdsObj->Flags |= CDS_OBJPROP_FLAGS_Searchable;
				 */

				/*
				 *  Containers supports OCM: upload content and OCM: create child container,
				 *  DlnaManaged OCM: detroy item bit does not apply to containers
				 */
				cdsObj->DlnaManaged =	CDS_DlnaManaged_UploadContent |
										CDS_DlnaManaged_CreateChildContainer;

				cdsObj->TypeObject.Container.ChildCount = 0;


				/*
				 *	Containers support OCM: content transfer must support <upnp:createClass> tag
				 *  This implemetation suppot audio, video and image items
				 */

				if(strncmp(cdsObj->ID, "0", strlen(cdsObj->ID))!=0)
				{
					cdsObj->TypeObject.Container.ChildCount = 3;

					/* set audioitem */
					cdsObj->TypeObject.Container.CreateClass = (struct CdsCreateClass*) malloc(sizeof(struct CdsCreateClass));
					memset(cdsObj->TypeObject.Container.CreateClass, 0, sizeof(struct CdsCreateClass));
					cdsObj->TypeObject.Container.CreateClass->IncludeDerived=1;
					cdsObj->TypeObject.Container.CreateClass->MediaClass = CDS_MEDIACLASS_AUDIOITEM;

					/* set iamgeitem */
					cdsObj->TypeObject.Container.CreateClass->Next = (struct CdsCreateClass*) malloc(sizeof(struct CdsCreateClass));
					memset(cdsObj->TypeObject.Container.CreateClass->Next, 0, sizeof(struct CdsCreateClass));
					cdsObj->TypeObject.Container.CreateClass->Next->IncludeDerived=1;
					cdsObj->TypeObject.Container.CreateClass->Next->MediaClass = CDS_MEDIACLASS_IMAGEITEM;

					/* set videoitem */
					cdsObj->TypeObject.Container.CreateClass->Next->Next = (struct CdsCreateClass*) malloc(sizeof(struct CdsCreateClass));
					memset(cdsObj->TypeObject.Container.CreateClass->Next->Next, 0, sizeof(struct CdsCreateClass));
					cdsObj->TypeObject.Container.CreateClass->Next->Next->IncludeDerived=1;
					cdsObj->TypeObject.Container.CreateClass->Next->Next->MediaClass = CDS_MEDIACLASS_VIDEOITEM;

					/* set container */
					cdsObj->TypeObject.Container.CreateClass->Next->Next->Next = (struct CdsCreateClass*) malloc(sizeof(struct CdsCreateClass));
					memset(cdsObj->TypeObject.Container.CreateClass->Next->Next->Next, 0, sizeof(struct CdsCreateClass));
					cdsObj->TypeObject.Container.CreateClass->Next->Next->Next->IncludeDerived=1;
					cdsObj->TypeObject.Container.CreateClass->Next->Next->Next->MediaClass = CDS_MEDIACLASS_CONTAINER;
				}
			}

			/*
			 *	This is a temp file, if the temp file was created from an upload request,
			 *	then the upload is incomplete, and we just expose the importUri value.
			 *  Since this is a file system, the only way to associate the importUri value
			 *  is faking it by regenerating the importUri value based on how the MSA object generated it
			 *  In a database backend, the application would call the MSA_ForCreateObjectResponse_AcceptUpload() method
			 *  which returns a request object, and you then call MSA_GetImportUri() to find out the importUri value and
			 *  store that in the database for the CDS item.
			 */

			//	/*
		}



		/*
		 *	MUSTDO: Track the latest date as a means
		 *	for deriving the updateID for this entry
		 */

		/*
		 *	Track the number of entries we've processed.
		 */
		qs->ProcessIndex++;
		qs->CdsObjectType = FSE_CDS_NORMAL;

		/*
		 *	free temp storage
		 */
		if (pi.Ext != NULL) { free (pi.Ext); pi.Ext = NULL; pi.ExtLen = 0; }
		if (pi.FullPath != NULL) { free (pi.FullPath); pi.FullPath = NULL; pi.FullPathLen = 0; }
		if (pi.Name != NULL) { free (pi.Name); pi.Name = NULL; pi.NameLen = 0; }
		if (pi.NameNoExt != NULL) { free (pi.NameNoExt); pi.NameNoExt = NULL; pi.NameNoExtLen = 0; }
		if (pi.ParentPath != NULL) { free (pi.ParentPath); pi.ParentPath = NULL; pi.ParentPathLen = 0; }
		if (pi.RelativeFullPath != NULL) { free (pi.RelativeFullPath); pi.RelativeFullPath = NULL; pi.RelativeFullPathLen = 0; } 
		if (pi.RelativeParent != NULL) { free (pi.RelativeParent); pi.RelativeParent = NULL; pi.RelativeParentLen = 0; }

	}

	if(fseRes->FileExtension!= NULL) {free(fseRes->FileExtension);}
	if(fseRes->LocalFilePath!= NULL) {free(fseRes->LocalFilePath);}
	free(fseRes);

	/*
	 *	finished populating the CDS Object for the file path, let's check if there's more files
	 *	in the directory to process, if so, update the QueringState object to prepare populating
	 *	the next CDS Object.
	 */
	if (qs->ProcessDirHandle != NULL)
	{
		while (ILibFileDir_GetDirNextFile(
						qs->ProcessDirHandle, 
						qs->FilePath, 
						fn,
						MAX_FILENAME_SIZE, 
						&(qs->ProcessFileSize)
						) != 0)
		{
			/*
			 *	A file was found. Let's see if it's something of interest.
			 */
			if (!((strcmp(fn, ".") == 0) || (strcmp(fn, "..") == 0)))
			{
				sprintf(qs->ProcessPath, "%s%s", qs->FilePath, fn);
				qs->ProcessObjectType = ILibFileDir_GetType(qs->ProcessPath);
				if(
					(qs->ProcessObjectType==ILibFileDir_Type_FILE) &&
					(ILibString_EndsWith(qs->ProcessPath, (int) strlen(qs->ProcessPath), EXTENSION_UPLOAD_TEMP, EXTENSION_UPLOAD_TEMP_LEN)!=0)
				)
				{
					/* treat incomplete upload files as CDS items. */
					qs->CdsObjectType = FSE_CDS_TEMPOPRARY;
				}

				/* return the expose value for the current populated CDS Object */
				return expose;
			}
		}

		#if defined (INCLUDE_FEATURE_PLAYSINGLE)
		/*
		*	SOLUTION_REFERENCE#3.6.3.11b
		*/

		/* If playsingle simulation is enabled, then it will use the last valid CDS item and create
		 * an additional CdsObject with a <res> element using the playsingle
		 */

		if(qs->CdsObjectType == FSE_CDS_NORMAL && qs->PlaySingleProcessPath[0] != '\0')
		{
			qs->CdsObjectType = FSE_CDS_PLAYSINGLE;
			/* replace the process path with the last valid CDS item process path. */
			{
				strcpy(qs->ProcessPath, qs->PlaySingleProcessPath);
				qs->ProcessObjectType = ILibFileDir_GetType(qs->ProcessPath);
				qs->PlaySingleProcessPath[0] = '\0';
			}
		}
		else
		{
			qs->CdsObjectType = FSE_CDS_NORMAL;
		#endif
			// no suitable entries were found, so mark it as being done
			qs->Flags |= FSE_FLAG_Done;
		#if defined (INCLUDE_FEATURE_PLAYSINGLE)
		}
		#endif
	}

	/* return the expose value for the current populated CDS Object */
	return expose;
}

void FSE_DestroyFSE(void *fseObj)
{
	struct FSE_State* fse = (struct FSE_State*) fseObj;
	free(fse->RootPath);
	free(fse->VirtualDirPath);
}

void* FSE_InitFSEState(void *chain, const char *rootDir, const char* virtualDir)
{
	struct FSE_State* fse = (struct FSE_State*) malloc (sizeof(struct FSE_State));
	int i;
	char* tempPtr;

	memset(fse, 0, sizeof(struct FSE_State));

	fse->Destroy = FSE_DestroyFSE;
	fse->RootPath = (char*)malloc((int)strlen(rootDir) + 2);	/* add 2 extra bytes for delimiter and null */
	strcpy(fse->RootPath, rootDir);
	fse->RootPathLen = (int) strlen(fse->RootPath);
	fse->VirtualDirPath = (char*)malloc((int)strlen(virtualDir) + 2);
	strcpy(fse->VirtualDirPath, virtualDir);
	fse->VirtualDirPathLen = (int) strlen(fse->VirtualDirPath);

	fse->DirDelimiter = NULL;

	/*
	 *	Set fse->DirDelimiter
	 */
	for (i = 0; i < fse->RootPathLen; i++)
	{
		if (FSE_BACKSLASH_CHR == fse->RootPath[i])
		{
			fse->DirDelimiter = FSE_BACKSLASH_STR;
			fse->DirDelimiterChr = FSE_BACKSLASH_CHR;
			fse->BadDirDelimiterChr = FSE_FORSLASH_CHR;
		}
		else if (FSE_FORSLASH_CHR == fse->RootPath[i])
		{
			fse->DirDelimiter = FSE_FORSLASH_STR;
			fse->DirDelimiterChr = FSE_FORSLASH_CHR;
			fse->BadDirDelimiterChr = FSE_BACKSLASH_CHR;
		}
	}
	if (fse->DirDelimiter == NULL)
	{
		fse->DirDelimiter = FSE_FORSLASH_STR;
		fse->DirDelimiterChr = FSE_FORSLASH_CHR;
		fse->BadDirDelimiterChr = FSE_BACKSLASH_CHR;
	}

	#ifdef WIN32
	fse->DirDelimiter = FSE_BACKSLASH_STR;
	fse->DirDelimiterChr = FSE_BACKSLASH_CHR;
	fse->BadDirDelimiterChr = FSE_FORSLASH_CHR;
	#endif

	/*
	 *	Ensure all directory delimiters are consistent.
	 */

	tempPtr = ILibString_Replace(fse->RootPath, fse->RootPathLen, &fse->BadDirDelimiterChr, 1, &fse->DirDelimiterChr, 1);
	strcpy(fse->RootPath, tempPtr);
	free(tempPtr);

	/*
	 *	Appropriately acquire the root path with a trailing delimiter.
	 */
	if (fse->RootPath[fse->RootPathLen-1] != fse->DirDelimiterChr)
	{
		/* no traling delimiter, so go ahead and add one */
		fse->RootPath[fse->RootPathLen] = fse->DirDelimiterChr;
		fse->RootPathLen++;
		fse->RootPath[fse->RootPathLen] = '\0';
	}

	ILibAddToChain(chain, fse);

	return fse;
}

unsigned int FSE_HandleOnAcquireUpdateIDAndCleanup(void *fseObj, struct MSA_CdsQuery *cdsQuery, void *onQueryObj)
{
	struct FSE_QueryState *qs = (struct FSE_QueryState*) onQueryObj;
	unsigned int retVal = qs->ProcessUpdateID;

	/*
	 *	We're done, so clean up our qs object and return an updateID.
	 */

	if (qs->ProcessDirHandle != NULL)
	{
		ILibFileDir_CloseDir(qs->ProcessDirHandle);
	}

	free(qs->FilePath);
	free(qs);

	return retVal;
}

void FSE_GetResourceMetadata(void *fseObj, void *onQueryObj, /*INOUT*/ struct FSE_ResourceMetadata *res)
{
	struct FSE_State* fse = (struct FSE_State*) fseObj;
	struct FSE_QueryState *qs = (struct FSE_QueryState*) onQueryObj;
	int dotPos, slashPos;

	/*
	 *	Return the fullpath name and the file size.
	 */
	if (res != NULL)
	{
		res->LocalFilePathLen = (int) strlen(qs->ProcessPath);
		res->LocalFilePath = (char*) malloc (res->LocalFilePathLen + 1);
		memcpy(res->LocalFilePath, qs->ProcessPath, res->LocalFilePathLen + 1);
		res->FileLength = qs->ProcessFileSize;

		slashPos = ILibString_LastIndexOf(res->LocalFilePath, res->LocalFilePathLen, fse->DirDelimiter, (int) strlen(fse->DirDelimiter));
		dotPos = ILibString_LastIndexOf(res->LocalFilePath, res->LocalFilePathLen, ".", 1);

		if (qs->ProcessObjectType == FSE_OT_File)
		{
			res->Flags |= FSE_RMF_IsFile;
			if ((dotPos > 0) && (dotPos > slashPos))
			{
				res->FileExtensionLen = res->LocalFilePathLen - dotPos;
				res->FileExtension = (char*) malloc(res->FileExtensionLen + 1);
				memcpy(res->FileExtension, res->LocalFilePath + dotPos, res->FileExtensionLen + 1);
			}
		}
		else
		{
			res->Flags |= FSE_RMF_IsDirectory;
		}
	}
}
int FSE_LocalFilePathToRelativePath(void *fseObj, const char* localFilePath, int localFilePathLen, char **relativePath)
{
	struct FSE_State* fse = (struct FSE_State*) fseObj;
	char *lfp = ILibString_Replace(localFilePath,localFilePathLen,"\\",1,"/",1);
	int RetVal = localFilePathLen - fse->RootPathLen;

	*relativePath = (char*)malloc(RetVal+1);
	memcpy(*relativePath,lfp+fse->RootPathLen,localFilePathLen-fse->RootPathLen);
	(*relativePath)[RetVal] = 0;

	free(lfp);
	return(RetVal);
}
int FSE_RelativePathToLocalFilePath(void *fseObj, const char *relativePath, int relativePathLen, char *localFilePath)
{
	struct FSE_State* fse = (struct FSE_State*) fseObj;
	int RetVal;
	char *tempString;

	tempString = ILibString_Replace(relativePath,relativePathLen,"/",1,"\\",1);
	relativePath = (const char*)tempString;
	if(ILibString_StartsWith(relativePath,relativePathLen,"\\",1)!=0)
	{
		relativePath += 1;
		relativePathLen -= 1;
	}
	
	RetVal = relativePathLen+fse->RootPathLen;
	memcpy(localFilePath,fse->RootPath,fse->RootPathLen);
	memcpy(localFilePath+fse->RootPathLen,relativePath,relativePathLen);
	localFilePath[RetVal]=0;
	free(tempString);
	return(RetVal);
}

#if defined(INCLUDE_FEATURE_UPLOAD)

int FSE_HandleDestroyObject(void *fseObj, struct CdsObject *destroyCdsObject)
{
	struct FSE_State* fse = (struct FSE_State*)fseObj;
	char* rootPath = NULL;
	char* id = NULL;
	int retVal, i, len = 0;
	char* file = NULL;
	char* tempFile = NULL;

	rootPath = fse->RootPath;
	if(destroyCdsObject->ID != NULL)
	{
		id = DecodeFromUTF8(destroyCdsObject->ID);

		len = (int)strlen(id+2) + (int)strlen(rootPath); // skip the 0/

		file = malloc(len + 1);
		strcpy(file, rootPath);
		strcat(file, id+2);
		free(id);

		for(i = 0; i < len; i++)
		{
			if(file[i] == '/')
			{
				file[i] = '\\';
			}
		}

		if(file[len-1] == '\\')
		{
			file[len-1] = '\0';
		}

		if(ILibFileDir_GetType(file) == FSE_OT_Directory)
		{
			retVal = ILibFileDir_DeleteDir(file);
		}
		else if(ILibFileDir_GetType(file) == FSE_OT_File)
		{
			retVal = ILibFileDir_DeleteFile(file);
			/* file deleted, also try delete the temp file if there's one */
			tempFile = (char*) malloc(strlen(file) + EXTENSION_UPLOAD_TEMP_LEN + 1);
			strcpy(tempFile, file);
			strcat(tempFile, EXTENSION_UPLOAD_TEMP);
			ILibFileDir_DeleteFile(tempFile);
		}
		else
		{
			/* file does not exit, maybe it is refering to the tmp file, try delete that */
			tempFile = (char*) malloc(strlen(file) + EXTENSION_UPLOAD_TEMP_LEN + 1);
			strcpy(tempFile, file);
			strcat(tempFile, EXTENSION_UPLOAD_TEMP);
			retVal = ILibFileDir_DeleteFile(tempFile);
		}
	}
	else
	{
		retVal = -1;
	}

	if(file!=NULL) free(file);
	if(tempFile!=NULL) free(tempFile);
	return retVal;
}

int FSE_HandleCreateObject(void *fseObj, struct MSA_CdsCreateObj *createObjArg, struct DLNAProtocolInfo* protocolInfo, struct CdsObject *newCdsObject)
{

	struct FSE_State* fse = (struct FSE_State*)fseObj;

	char* id = NULL;
	char* parentId = NULL;
	char* title = NULL;
	int size;
	int retVal = 0;
	int parentIdLen = 0;

	if(newCdsObject != NULL)
	{
		parentId = DecodeFromUTF8(newCdsObject->ParentID);
		parentIdLen = (int) strlen(parentId);

		if((newCdsObject->MediaClass & CDS_CLASS_MASK_OBJECT_TYPE) == CDS_CLASS_MASK_CONTAINER)
		{
			/* this is a container, so the file system will create a new folder */
			char* rootPath = NULL;
			int i, len = 0;
			char* folder = NULL;

			if(newCdsObject->Title != NULL)
			{

				rootPath = fse->RootPath;
				title = DecodeFromUTF8(newCdsObject->Title);
				len = (int)strlen(title) + (int)strlen(rootPath) + parentIdLen;

				folder = malloc(len + 1);
				strcpy(folder, rootPath);
				if(ILibString_StartsWith(parentId, parentIdLen, "0\\", 2)!=0 && parentIdLen>2)
				{
					strcat(folder, parentId + 2);		// skip directory 0
				}
				else if(strnicmp(parentId, CDS_STRING_DLNA_ANYCONTAINER, CDS_STRING_DLNA_ANYCONTAINER_LEN)==0)
				{
					// ToDo: how to handle AnyContainer uploads
					// for now it reset to the root directory of the CDS.
					free(parentId);
					parentId = ILibString_Copy("0\\", -1);
				}

				strcat(folder, title);

				for(i = 0; i < len; i++)
				{
					if(folder[i] == '/')
					{
						folder[i] = '\\';
					}
				}

				retVal = ILibFileDir_CreateDir(folder);

				free(folder);
			}
		}
		else
		{
			/* this is an item, the file system will map the item to a local file, and return the corresponding object ID */
			char* rootPath = NULL;
			char* fileExtension = NULL;
			char* extension = NULL;
			int i, len = 0;
			char* file = NULL;
			char* tmp = NULL;

			rootPath = fse->RootPath;
			if(newCdsObject->Title != NULL)
			{
				title = DecodeFromUTF8(newCdsObject->Title);
			}
			else
			{
				int num = rand() % 1000000000;
				title = (char*) malloc(15);
				sprintf(title, "file%d", num);
			}
			/* we cannot rely on the title of the Cds Item to determine its media type, but rather its DLNA Profile */

			switch(newCdsObject->MediaClass & CDS_CLASS_MASK_MAJOR)
			{
				case CDS_CLASS_MASK_MAJOR_AUDIOITEM:

					if(strcmp(protocolInfo->Profile, DLNAPROFILE_LPCM)==0)
					{
						extension = EXTENSION_AUDIO_LPCM;
					}
					else if(strcmp(protocolInfo->Profile, DLNAPROFILE_MP3)==0)
					{
						extension = EXTENSION_AUDIO_MPEG;
					}
					else if(strcmp(protocolInfo->Profile, DLNAPROFILE_AMR_3GPP)==0)
					{
						extension = EXTENSION_AUDIO_3GPP;
					}
					else if(strcmp(protocolInfo->Profile, DLNAPROFILE_AAC_ISO_320)==0)
					{
						extension = EXTENSION_AUDIO_AAC;
					}
					else if(strcmp(protocolInfo->Profile, DLNAPROFILE_WMABASE)==0)
					{
						extension = EXTENSION_AUDIO_WMA;
					}
					else
					{
						/* not supported */

					}
					break;
				case CDS_CLASS_MASK_MAJOR_IMAGEITEM:
					if(strcmp(protocolInfo->Profile, DLNAPROFILE_JPEG_SM)==0)
					{
						extension = EXTENSION_IMAGE_JPG;
					}
					else if(strcmp(protocolInfo->Profile, DLNAPROFILE_JPEG_MED)==0)
					{
						extension = EXTENSION_IMAGE_JPG;
					}
					else if(strcmp(protocolInfo->Profile, DLNAPROFILE_JPEG_LRG)==0)
					{
						extension = EXTENSION_IMAGE_JPG;
					}
					else if(strcmp(protocolInfo->Profile, DLNAPROFILE_PNG_LRG)==0)
					{
						extension = EXTENSION_IMAGE_PNG;
					}
					else
					{
						/* not supported */
					}
					break;
				case CDS_CLASS_MASK_MAJOR_VIDEOITEM:
					if(strcmp(protocolInfo->Profile, DLNAPROFILE_MPEG_PS_NTSC)==0)
					{
						extension = EXTENSION_VIDEO_MPEG2;
					}
					else if(strcmp(protocolInfo->Profile, DLNAPROFILE_AVC_MP4_BL_CIF15_AAC_520)==0)
					{
						extension = EXTENSION_VIDEO_AAC;
					}
					else if(strcmp(protocolInfo->Profile, DLNAPROFILE_MPEG_PS_NTSC)==0)
					{
						extension = EXTENSION_VIDEO_MPEG2;
					}
					else if(strcmp(protocolInfo->Profile, DLNAPROFILE_MPEG4_P2_ASF_SP_G726)==0)
					{
						extension = EXTENSION_VIDEO_ASF;
					}
					else if(strcmp(protocolInfo->Profile, DLNAPROFILE_WMVMED_BASE)==0)
					{
						extension = EXTENSION_VIDEO_WMV;
					}
					else
					{
						/* not supported */
					}
					break;
			}

			if(extension == NULL)
			{
				/* no valid extension found, not a valid media item */
				return -1;
			}

			/* replace file extension with the valid extension found */
			fileExtension = FilePathToFileExtension(title, 0);

			if(fileExtension != NULL)
			{
				tmp = title;

				/* replace old title with new title with correct extension */
				title = ILibString_Replace(
					title,
					(int) strlen(title),
					fileExtension,
					(int) strlen(fileExtension),
					extension,
					(int) strlen(extension));

				free(fileExtension);
				free(tmp);
			}
			else
			{
				tmp = (char*) malloc((int) strlen(title) + (int) strlen(extension) + 1);
				strcpy(tmp, title);
				strcat(tmp, extension);
				free(title);
				title = tmp;
			}

			/* replace old title with UTF-8 encoded title */
			if (newCdsObject->DeallocateThese & CDS_ALLOC_Title)
			{
				free(newCdsObject->Title);
			}
			newCdsObject->Title = EncodeToUTF8(title);

			len = (int)strlen(title) + (int)strlen(rootPath) + parentIdLen;

			file = malloc(len + 1);
			strcpy(file, rootPath);
			if(ILibString_StartsWith(parentId, parentIdLen, "0\\", 2)!=0 && parentIdLen>2)
			{
				strcat(file, parentId + 2);		// skip directory 0
			}
			else if(strnicmp(parentId, CDS_STRING_DLNA_ANYCONTAINER, CDS_STRING_DLNA_ANYCONTAINER_LEN)==0)
			{
				// ToDo: how to handle AnyContainer uploads
				// for now it reset to the root directory of the CDS.
				free(parentId);
				parentId = ILibString_Copy("0\\", -1);
			}
			strcat(file, title);

			for(i = 0; i < len; i++)
			{
				if(file[i] == '/')
				{
					file[i] = '\\';
				}
			}

			/* Finds out if there's already a uploaded object */
			tmp = (char*) malloc(strlen(file) + EXTENSION_UPLOAD_TEMP_LEN + 1);
			strcpy(tmp, file);
			strcat(tmp, EXTENSION_UPLOAD_TEMP);
			if(ILibFileDir_GetType(tmp) == FSE_OT_NoExist)
			{
				/* temp file does not exists, no file is currently being uploaded */
				retVal = 0;

				/* overwriting the file, remove the old file in order to avoid duplicates in CDS*/
				ILibFileDir_DeleteFile(file);
				newCdsObject->Source = ILibString_Copy(tmp,-1);
			}
			else
			{
				retVal = -1;
			}
			free(file);
			free(tmp);
		}
	}

	// replace requested object ID with the correct object ID;
	if ((newCdsObject->ParentID != NULL) && (newCdsObject->DeallocateThese & CDS_ALLOC_ParentID))
	{
		// free previous parentId
		free(newCdsObject->ParentID);
	}
	newCdsObject->ParentID = EncodeToUTF8(parentId);

	if((newCdsObject->ID != NULL) && (newCdsObject->DeallocateThese & CDS_ALLOC_ID))
	{
		free(newCdsObject->ID);
	}

	size = parentIdLen + (int) strlen(title) + 2;
	id = (char*) malloc (size);
	strcpy(id, parentId);
	if(ILibString_EndsWith(parentId, parentIdLen, "\\", 1)!=0)
	{
		strcat(id, "\\");
	}
	strcat(id, title);
	newCdsObject->ID = EncodeToUTF8(id);
	newCdsObject->DeallocateThese |= CDS_ALLOC_ParentID;
	newCdsObject->DeallocateThese |= CDS_ALLOC_ID;

	if(id!=NULL) free(id);
	if(parentId!=NULL) free(parentId);
	if(title!=NULL) free(title);

	return retVal;
}

#endif
