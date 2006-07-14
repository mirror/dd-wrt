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
 * $Workfile: CdsDidlSerializer.h
 *
 *
 *
 */

#ifndef _CDS_DIDL_SERIALIZER_H
#define _CDS_DIDL_SERIALIZER_H
#include "CdsObject.h"

/*! \file CdsDidlSerializer.h 
	\brief Provides methods useful for serializing CDS objects into DIDL-Lite.
	Also provides methods for useful for using CDS objects on a DMS.
*/

/*! \defgroup CdsObjectToDidl CDS Helper - DIDL-Lite Serializer
	\brief 
	This module provides the functionality for translating the 
	\ref CdsObject and \ref CdsResource instances into
	valid DIDL-Lite/XML strings. 

	\{
*/

/*!	\brief Enumeration of different types of syntax errors for a CDS object.
 
	\note All values must be negative as non-negative return codes in some
	methods indicate success.
 */
enum Errors_CdsToDidl
{
	/*! \brief media class has value of zero */
	Error_CdsToDidl_InvalidMediaClass		= -1,
	/*! \brief media class has undefined object type */
	Error_CdsToDidl_UndefinedObjectType	= -2,

	/*! \brief media class has undefined major type */
	Error_CdsToDidl_UndefinedMajorType	= -3,

	/*! \brief media class has undefined minor1 type */
	Error_CdsToDidl_UndefinedMinor1Type	= -4,
	
	/*! \brief media class has undefined minor2 type */
	Error_CdsToDidl_UndefinedMinor2Type	= -5,

	/*! \brief media class is container, but data has refID */
	Error_CdsToDidl_MismatchContainerRefID= -6,

	/*! \brief media objects cannot have an empty title */
	Error_CdsToDidl_EmptyTitle			= -7,

	/*! \brief media objects cannot have empty object ID values */
	Error_CdsToDidl_EmptyObjectID			= -8,

	/*! \brief media objects cannot have empty parent ID values. 
		Parents of root containers should be "-1". 
	*/
	Error_CdsToDidl_EmptyParentID			= -9,

	/*!	\brief LOGIC error - e.g. the code did not allocate enough memory */
	Error_CdsToDidl_CorruptedMemory		= -99
};

/*!	\brief Enumerates the optional fields that should be included in the response.

	You can bitwise-OR these values with to set bits in a bit_string
	to allow a compact and convenient representation of a 
	CDS:Browse/Search "filter" input parameter.
	
	\warning <b>MUSTDO:</b> Need to add support DLNA-recommended properties that are not implemented here.

	\note <b>TODO:</b>
	If you add new metadata properties to \ref CdsObject or \ref CdsResource, 
	then modify this enum so that each new property has a bit index.
 */
enum CdsFilterBits
{
	/*!	\brief include <b>dc:creator</b> metadata in the response */
	CdsFilter_Creator				= 0x00000001,		

	/*!	\brief include <b>\@childCount</b> metadata in the response */
	CdsFilter_ChildCount			= 0x00000002,

	/*!	\brief include <b>\@searchable</b> metadata in the response */
	CdsFilter_Searchable			= 0x00000004,

	/*!	\brief include <b>upnp:album</b> metadata in the response */
	CdsFilter_Album					= 0x00000008,

	/*!	\brief include <b>upnp:genre</b> metadata in the response */
	CdsFilter_Genre					= 0x00000010,

	/*!	\brief include the minimal <b>res\@protocolInfo</b> and URI value in the response */
	CdsFilter_Res					= 0x00000020,

	/*!	\brief Include the <b>res\@resolution</b> attribute in the response. Infers \ref CdsFilter_Res bit is true. */
	CdsFilter_ResResolution			= 0x00000040,	

	/*!	\brief Include the <b>res\@duration</b> attribute in the response. Infers \ref CdsFilter_Res bit is true. */
	CdsFilter_ResDuration			= 0x00000080,	

	/*!	\brief Include the <b>res\@bitrate</b> attribute in the response. Infers \ref CdsFilter_Res bit is true. */
	CdsFilter_ResBitrate			= 0x00000100,	

	/*!	\brief Include the <b>res\@colorDepth</b> attribute in the response. Infers \ref CdsFilter_Res bit is true. */
	CdsFilter_ResColorDepth			= 0x00000200,	

	/*!	\brief Include the <b>res\@size</b> attribute in the response. Infers \ref CdsFilter_Res bit is true. */
	CdsFilter_ResSize				= 0x00000400,	

	/*!	\brief Include the <b>res\@bitsPerSample</b> attribute in the response. Infers \ref CdsFilter_Res bit is true. */
	CdsFilter_ResBitsPerSample		= 0x00000800,

	/*!	\brief Include the <b>res\@sampleFrequency</b> attribute in the response. Infers \ref CdsFilter_Res bit is true. */
	CdsFilter_ResSampleFrequency	= 0x00001000,

	/*!	\brief Include the <b>res\@nrAudioChannels</b> attribute in the response. Infers \ref CdsFilter_Res bit is true. */
	CdsFilter_ResNrAudioChannels	= 0x00002000,

	/*!	\brief Include the <b>res\@protection</b> attribute in the response. Infers \ref CdsFilter_Res bit is true. */
	CdsFilter_ResProtection			= 0x00004000,

	/*	\brief include <b>res\@importUri</b> metadata in the response*/

	CdsFilter_ResImportUri			= 0x00008000,

	/*	\brief include <b>res\@resIfoFileUri</b> metadata in the response	*/

	CdsFilter_ResIfoFileUri			= 0x00010000,

	/*	\brief include <b>res\@resImportIfoFileUri</b> metadata in the response
	*/
	CdsFilter_ResImportIfoFileUri	= 0x00020000,

	/*	\brief include <b>res\@dlna:resumeUpload</b> metadata in the response
	*/
	CdsFilter_ResResumeUpload		= 0x00040000,

	/*	\brief include <b>res\@dlna:uploadedSize</b> metadata in the response
	*/
	CdsFilter_ResUploadedSize		= 0x00080000,

	/*	\brief include <b>res\@dlna:trackTotal</b> metadata in the response
	*/
	CdsFilter_ResTrackTotal			= 0x00100000,

	/*!	\brief include <b>upnp:channelName</b> metadata in the response 
	*/
	CdsFilter_ChannelName			= 0x00200000,

	/*!	\brief include <b>upnp:channelNr</b> metadata in the response 
	*/
	CdsFilter_ChannelNr				= 0x00400000,

	/*!	\brief include <b>dc:date</b> metadata in the response 
	*/
	CdsFilter_Date					= 0x00800000,

	/*!	\brief include <b>dlna:dlnaManaged</b> metadata in the response 
	*/
	CdsFilter_DlnaManaged			= 0x01000000,

	/*!	\brief include <b>dlna:containerType</b> metadata in the response 
	*/
	CdsFilter_DlnaContainerType		= 0x02000000,

	/*	\brief include <b>dlna:takeOut</b> metadata in the response
	*/
	CdsFilter_TakeOut				= 0x04000000,


	/*!	\brief DLNA-extension. This bit means the CDS must return &lt;res&gt; values
	 *	for all available IP/network addresses.
	 */
	CdsFilter_DhwgAllIp				= 0x08000000,

	/* \brief same as \ref CdsFilterBits::CdsFilter_DhwgAllIp */
	CdsFilter_DlnaAllIp				= 0x08000000,

	/*!	\brief Return all attributes for the &lt;res&gt; element.
	 */
	CdsFilter_ResAllAttribs = 
			CdsFilter_Res | 
			CdsFilter_ResResolution | 
			CdsFilter_ResDuration | 
			CdsFilter_ResBitrate | 
			CdsFilter_ResColorDepth | 
			CdsFilter_ResSize |
			CdsFilter_ResBitsPerSample |
			CdsFilter_ResSampleFrequency |
			CdsFilter_ResNrAudioChannels |
			CdsFilter_ResProtection |
			CdsFilter_ResImportUri |
			CdsFilter_ResIfoFileUri |
			CdsFilter_ResImportIfoFileUri |
			CdsFilter_ResResumeUpload |
			CdsFilter_ResUploadedSize |
			CdsFilter_ResTrackTotal
};

/*!	\brief Provides an enumeration of the properties that can be used for sorting and searching.
 
	Applications usually set fields on an unsigned int and call
	\ref CDS_ConvertBitStringToCsvString() to convert a set
	of properties into a comma-separated list of properties.

	Generally, the properties listed here are the ones that a
	DMS implementation should consider for indexing in a relational database.

 	\warning The specified values in this list MUST correspond with values in \ref CdsFilterBits.
 */
enum CdsSortableSearchableFields
{
	/*!	\brief Include <b>dc:title</b> in the set of properties. */
	CdsSearchSort_Title			= 0x00000001,

	/*!	\brief Include <b>dc:creator</b> in the set of properties. */
	CdsSearchSort_Creator		= 0x00000002,

	/*!	\brief Include <b>upnp:class</b> in the set of properties. */
	CdsSearchSort_Class			= 0x00000004,

	/*!	\brief Include <b>upnp:album</b> in the set of properties. */
	CdsSearchSort_Album			= 0x00000400,

	/*!	\brief Include <b>upnp:genre</b> in the set of properties. */
	CdsSearchSort_Genre			= 0x00000800,

	/*!	\brief Include all of the properties. */
	CdsSearchSort_ALL = CdsSearchSort_Title | CdsSearchSort_Creator | CdsSearchSort_Class | CdsSearchSort_Album | CdsSearchSort_Genre
};

/*!	\brief Returns a bit string (as an unsigned int) 
	where each bit indicates if a supported optional metadata field 
	should be included in a DIDL-Lite response (to a CDS:Browse/CDSSearch) request. 

	\warning <b>TODO:</b> The implementation of this method matches the supported metadata 
	described in \ref CdsObject and \ref CdsResource. 
	If you add new metadata properties to \ref CdsObject or \ref CdsResource, 
	then this method needs to be modified so that the new properties are 
	mapped to bits in the returned value.

	\param[in] filter Comma-delimited string of metadata filters. If null, then the return value is zero.
	If "*", then all bits are set to 1.
	\returns The bitstring, where each bit is accessible with \ref CdsFilterBits.
 */
unsigned int CDS_ConvertCsvStringToBitString(const char *filter);

/*!	\brief Returns the number of bytes needed to store the comma-separated form of
 	the specified filter in bitstring form.
 
 	\warning <b>TODO:</b> If you add new metadata properties to 
	\ref CdsObject or \ref CdsResource, then this method
 	needs to be modified to account for the new metadata properties.
 
 	\param[in] bit_string bitwise OR'd values of \ref CdsFilterBits or 
	\ref CdsSortableSearchableFields

	\return The number of bytes needed to properly store the corresponding bit string.
 */
int CDS_GetCsvLengthFromBitString(unsigned int bit_string);

/*!	\brief Translates \a filter into a comma-separated value list in string form.
 
 	\warning <b>TODO:</b> If you add new metadata properties to 
	\ref CdsObject or \ref CdsResource, then this method
 	needs to be modified to print the proper DIDL-Lite names of the 
	metadata properties.
 
 	\param[in,out] csv_string Store the comma-separated list in this argument.
 	\param[in] bit_string Bitwise OR'd values of \ref CdsFilterBits or \ref CdsSortableSearchableFields.
 	\returns \a csv_string
 */
char* CDS_ConvertBitStringToCsvString(char *csv_string, unsigned int bit_string);

/*!	\brief Transforms a CDS object and its &lt;res&gt; elements into 
	a DIDL-Lite &lt;item&gt; or &lt;container&gt; element.
	
	The returned value represents the provided CDS object in \a media_obj, 
	including the linked list of resources obtained from 
	\ref CdsObject::Res. The method also applies metadata filtering,
	such that only the requested metadata specified in \a filter
	is included in the result.

 	\warning
	<b>TODO:</b> If you add new metadata properties to 
	\ref CdsObject or \ref CdsResource, then this method
 	needs to be modified to print the new metadata properties only when the
 	filter bitstring indicates that those properties should be included in 
	the results.

	\param[in] cds_obj The CDS object to serialize into DIDL-Lite XML form.
	All string metadata on \a cds_obj (as well as in the \a cds_obj->Res 
	linked list) must be UTF-8 encoded.

	\param[in] cds_obj_metadata_escaped
	- If nonzero, then all of the metadata on \a cds_obj is already properly XML-escaped. 
	- If zero, then the method will employ XML-escaping on string values to ensure proper encoding.

	\param[in] filter A bit string of desired metadata fields to include 
	in the DIDL-Lite. This value is usually obtained from 
	\ref CDS_ConvertCsvStringToBitString().

	\param[in] include_header_footer If nonzero, then the returned string 
	should be encapsulated in a &lt;DIDL-Lite&gt; element document tag.

	\param[in] out_error_or_didl_len
	- If the return value is non-NULL, then this is the length of the returned string.
	- If the returned value is NULL, this value will contain a negative error code
	that can be cast to a \ref Errors_CdsToDidl.
	\returns The &lt;item&gt; or &lt;container&gt; element that 
	represents \a cds_obj. Use \a free() to deallocate this data when the
	application no longer needs it.
 */
char* CDS_SerializeObjectToDidl (struct CdsObject *cds_obj, int cds_obj_metadata_escaped, unsigned int filter, int include_header_footer, int *out_error_or_didl_len);

char* CDS_SerializeObjectToDidlUnescaped (struct CdsObject *cds_obj, int cds_obj_metadata_escaped, unsigned int filter, int include_header_footer, int *out_error_or_didl_len);

/*!	\brief Creates \ref CdsObject from an XML node representing a CDS object.

	\param[in] node The XML data that represents our CDS object
	\param[in] attribs The XML attributes of the CDS object (i.e. item or container attributes).
	\param[in] is_item A nonzero indicates that \a node represents a CDS item.
	\param[in] range_start The actual string data for \a node starts at this memory address.
	\param[in] range_end: The actual string data for \a node ends at this memory address.
 */
struct CdsObject* CDS_DeserializeDidlToObject(struct ILibXMLNode *node, struct ILibXMLAttribute *attribs, int is_item, const char *range_start, const char *range_end);


/*!	\brief Clones a media object with its resources. 

	\warning
	- If (\a clone_this->CpInfo ==NULL), then you can
	use \ref CDS_DestroyObject() to free the cloned media object.
	Generally, you should use 
	\ref CDS_ObjRef_Release() to free things because
	\ref CDS_DestroyObject() is deprecated.

	\param[in] clone_this The CDS object to clone.
	\returns A deep copy of the CDS object.

	\warning <b>TODO</b>If you made changes to the supported metadata, then need to modify CDS_CloneMediaObject.
*/
struct CdsObject* CDS_CloneMediaObject(const struct CdsObject *clone_this);

/*!	\brief Adds a reference to the CDS object. 

	This ensures that the object is not deallocated unless
	\ref CDS_ObjRef_Add is not called an equal number of times.

	\warning Do not use this method in conjunction with 
	\ref CDS_DestroyObject() or \ref CDS_DestroyResources().

	\param[in] ref_this The CDS object to add a reference to.
*/
void CDS_ObjRef_Add(struct CdsObject *ref_this);

/*!	\brief Releases a reference to the CDS object.

	Use this to "clean up" from an earlier call to \ref CDS_ObjRef_Add()
	or to destroy a clone of a CDS object.

	\warning Do not use this method in conjunction with 
	\ref CDS_DestroyObject() or \ref CDS_DestroyResources().

	\param[in] release_this The CDS object to decrement a reference from.
 */
void CDS_ObjRef_Release(struct CdsObject *release_this);

/*! \} */

#endif
