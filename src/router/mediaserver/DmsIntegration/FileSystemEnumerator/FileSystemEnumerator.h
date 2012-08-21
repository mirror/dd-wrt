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
 * $Workfile: FileSystemEnumerator.h
 *
 *
 *
 */

#ifndef _FILESYSTEM_ENUMERATOR_H
#define _FILESYSTEM_ENUMERATOR_H

#include "CdsErrors.h"
#include "CdsObject.h"
#include "MediaServerAbstraction.h"

#if defined(INCLUDE_FEATURE_UPLOAD)
#include "DLNAProtocolInfo.h"
#endif

#define MAX_FILENAME_SIZE 1024

/*!	\brief Defines the temp file extension used in upload request.
*/
#define	EXTENSION_UPLOAD_TEMP			".tmp"

/*!	\brief Defines the temp file extension length used in upload request.
*/
#define EXTENSION_UPLOAD_TEMP_LEN		4


enum FSE_ResourceMetadataFlags
{
	FSE_RMF_IsFile				= 0x0001,
	FSE_RMF_IsDirectory			= 0x0002,
	FSE_RMF_IsPlaylist			= 0x0004
};

/*
 *	Must match values returned by ILibFileDir_GetFileDirType()
 */
enum FSE_CdsObjectType
{
	FSE_CDS_NORMAL = 0,
	FSE_CDS_TEMPOPRARY = 1,
	FSE_CDS_PLAYSINGLE = 2
};

/*
 *	TODO: You can customize this struct to match the
 *	capabilities of the metadata store. As a note,
 *	there is no requirement for every metadata
 *	store to have the same struct. Although
 *	the FSE returns only a local file path
 *	and a filesize, this information could be sent
 *	to other modules (such as a transcoding engine)
 *	that can provide additional CdsResource
 *	objects for a media object.
 */
struct FSE_ResourceMetadata
{
	int Flags;					/* mapped by (enum FSE_ResourceMetadataFlags) */
	char *LocalFilePath;
	int LocalFilePathLen;
	char *FileExtension;
	int FileExtensionLen;
	unsigned int FileLength;
};

/*
 *	This module provides the interface for enumerating a file system as if it were
 *	a metadata store.
 */

/*
 *	Starts the process for handling a CDS query.
 *
 *	fseObj				: object from FSE_InitFSEState().
 *
 *	cdsQuery			: the details of the CDS metadata query
 *
 *	filter				: a bit string indicating which fields to return;
 *						: bit string is mapped by 'enum CdsFilterBits'
 *
 *	cdsErrorCode		: contains a CDS error code (enum Enum_CdsErrors)
 *						: in case the query cannot be resolved
 *
 *	Returns void* object representing the query, as understood by the FSE.
 */
void* FSE_HandleQuery(void* fseObj, struct MSA_CdsQuery *cdsQuery, unsigned int filter, /*OUT*/ enum Enum_CdsErrors *cdsErrorCode);

/*
 *	Used to obtain metadata for a CdsObject.
 *	This method is called after FSE_HandleQuery() and 
 *	before FSE_HandleOnAcquireUpdateIDAndCleanup().
 *
 *	fseObj				: object from FSE_InitFSEState().
 *
 *	cdsQuery			: the CDS metadata query that was sent to FSE_HandleQuery()
 *
 *	onQueryObj			: the object obtained from FSE_HandleQuery()
 *
 *	filter				: a bit string indicating which fields to return;
 *						: bit string is mapped by 'enum CdsFilterBits'
 *
 *	cdsObj				: allocated CdsObject;
 *						: fields will be initialized to zero
 *
 *	Returns nonzero if metadata was applied to cdsOb. Return value of zero indicates
 *	that there are no more objects to process.
 */
int FSE_HandleProcessObject(void *fseObj, struct MSA_CdsQuery *cdsQuery, void *onQueryObj, unsigned int filter, /*INOUT*/ struct CdsObject *cdsObj);

/*
 *	Used to obtain the 'UpdateID' out parameter value for a CDS response.
 *	This method should at the very end of responding to a CDS query.
 *
 *	fseObj				: object from FSE_InitFSEState().
 *
 *	cdsQuery			: the CDS metadata query that was sent to FSE_HandleQuery()
 *
 *	onQueryObj			: the object obtained from FSE_HandleQuery()
 *
 *	Returns a value to populate the 'UpdateID' parameter in the
 *	CDS response.
 */
unsigned int FSE_HandleOnAcquireUpdateIDAndCleanup(void *fseObj, struct MSA_CdsQuery *cdsQuery, void *onQueryObj);

void* FSE_InitFSEState(void* chain, const char *rootDir, const char* virtualDir);

/*
 *	Returns metadata that is useful creating a CdsResource object.
 *	This method is expected to be called after FSE_HandleQuery()
 *	and before FSE_HandleOnAcquireUpdateIDAndCleanup(). Generally,
 *	for every call to FSE_HandleProcessObject() there should be a call
 *	to FSE_GetResourceMetadata().
 *
 *	fseObj				: object from FSE_InitFSEState().
 *
 *	onQueryObj			: the object obtained from FSE_HandleQuery()
 *
 *	res					: allocated FSE_ResourceMetadata; struct will be memset to zero
 *						: Caller needs to call free() on char* fields to free the 
 *						: allocated memory.
 */
void FSE_GetResourceMetadata(void *fseObj, void *onQueryObj, /*INOUT*/ struct FSE_ResourceMetadata *res);

int FSE_RelativePathToLocalFilePath(void *fseObj, const char *relativePath, int relativePathLen, char *localFilePath);

int FSE_LocalFilePathToRelativePath(void *fseObj, const char* localFilePath, int localFilePathLen, char **relativePath);

#if defined(INCLUDE_FEATURE_UPLOAD)

int FSE_HandleCreateObject(void *fseObj, struct MSA_CdsCreateObj *createObjArg, struct DLNAProtocolInfo* protocolInfo, struct CdsObject *newCdsObject);

int FSE_HandleDestroyObject(void* fseObj, struct CdsObject *destroyCdsObject);

#endif

#endif
