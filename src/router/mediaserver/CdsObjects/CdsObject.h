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
 * $Workfile: CdsObject.h
 *
 *
 *
 */

#ifndef _CDS_OBJECT_H
#define _CDS_OBJECT_H
#if defined(_WIN32_WCE)
#include <winsock.h>
#endif

#include "ILibParsers.h"
#include "CdsMediaClass.h"

/*! \file CdsObject.h
	\brief Provides struct representations of CDS objects and &lt;res&gt; elements.
*/

/*! \defgroup CdsObject CDS Helper - Objects and Resources
	\brief Provides minimal data structures to represent
 	CDS objects and their resource elements.

 	Vendors that want to increase the supported metadata elements
 	should modify these structures appropriately.

 	This library does not presume use specifically for control point
 	or device-implementation software.

	\warning <b>MUSTDO:</b>This module needs to be updated to support all normative
	DLNA properties.
	\{
*/

/*!	\brief Boolean-based properties for CDS items and containers.
*/
enum CdsObjectProperties
{
	/*!	\brief Indicates if the CDS object supports one or more
		DLNA <i>content management operations</i>.

		The <i>content management operations</i> include things like
		- uploading new CDS items to CDS containers,
		- creating new containers, and
		- destroying CDS items.
	*/
	CDS_OBJPROP_FLAGS_Restricted =			0x00000001,

	/*!	\brief Indicates if the CDS container will support a
		CDS:Search request on the CDS object.
	*/
	CDS_OBJPROP_FLAGS_Searchable =			0x00000002,
};

/*! \brief Boolean-based properties for res elements.
 */
enum CdsResourceProperties
{
	/*!	\brief Indicates if the &lt;res&gt; supports the 
		<i>resume content transfer</i> operation.
	*/
	CDS_RESPROP_FLAGS_ResumeUpload =		0x00000001,
};

/*!	\brief Bits used to represent bits on the \@dlnaManaged attribute.

	\warning If any of these bits are set, then
	\ref CDS_OBJPROP_FLAGS_Restricted must be false.
*/
enum CdsDlnaManagedBits
{
	/*!	\brief Indicates if the CDS container supports the
		<i>OCM:Upload Content</i> operation.
	*/
	CDS_DlnaManaged_UploadContent =			0x00000001,

	/*!	\brief Indicates if the CDS container supports the
		<i>OCM:Create Child Container</i> operation.
	*/
	CDS_DlnaManaged_CreateChildContainer =	0x00000002,

	/*!	\brief Indicates if the CDS container supports the
		<i>OCM:destroy item</i> operation.
	*/
	CDS_DlnaManaged_DestroyItem =			0x00000004,
};

/*!	\brief Boolean mapping so that applications can track which fields need
	to be deallocated on \ref CdsObject::DeallocateThese.
*/
enum CdsAllocatable
{
	/*!	\brief \ref CdsObject::ID needs to be deallocated.
	*/
	CDS_ALLOC_ID		= 0x0001,

	/*!	\brief \ref CdsObject::ParentID needs to be deallocated.
	*/
	CDS_ALLOC_ParentID	= 0x0002,

	/*!	\brief <i>CdsObject_Item::RefID</i> needs to be deallocated.
	*/
	CDS_ALLOC_RefID		= 0x0004,

	/*!	\brief \ref CdsObject::Title needs to be deallocated.
	*/
	CDS_ALLOC_Title		= 0x0008,

	/*!	\brief \ref CdsObject::Creator needs to be deallocated.
	*/
	CDS_ALLOC_Creator	= 0x0010,

	/*!	\brief The <i>Albums</i> property needs to be deallocated.

		This is usually an array of strings associated that can be found
		on one of the structs declared within \ref MajorTypes
		or \ref Minor1Types, or \ref Minor2Types.
	*/
	CDS_ALLOC_Album		= 0x0020,

	/*!	\brief The <i>Genres</i> property needs to be deallocated.

		This is usually an array of strings associated that can be found
		on one of the structs declared within \ref MajorTypes
		or \ref Minor1Types, or \ref Minor2Types.
	*/
	CDS_ALLOC_Genre		= 0x0040,

	/*!	\brief The <i>ChannelName</i> property needs to be deallocated.

		This is usually an array of strings associated that can be found
		on one of the structs declared within \ref MajorTypes
		or \ref Minor1Types, or \ref Minor2Types.
	*/
	CDS_ALLOC_ChannelName = 0x0080
};

/*!	\brief Boolean mapping so that applications can track which fields need
	to be deallocated on \ref CdsResource::Allocated.
*/
enum CdsResAllocatable
{
	/*!	\brief \ref CdsResource::Value needs to be deallocated.
	*/
	CDS_RES_ALLOC_Value			= 0x0001,
	/*!	\brief \ref CdsResource::ProtocolInfo needs to be deallocated.
	*/
	CDS_RES_ALLOC_ProtocolInfo	= 0x0002,

	/*!	\brief \ref CdsResource::Protection needs to be deallocated.
	*/
	CDS_RES_ALLOC_Protection = 0x0004,

	/*!	\brief \ref CdsResource::ImportUri needs to be deallocated.
	*/
	CDS_RES_ALLOC_ImportUri = 0x0008,

	/*!	\brief \ref CdsResource::IfoFileUri needs to be deallocated.
	*/
	CDS_RES_ALLOC_IfoFileUri = 0x0010,

	/*!	\brief \ref CdsResource::ImportIfoFileUri needs to be deallocated.
	*/
	CDS_RES_ALLOC_ImportIfoFileUri = 0x0020,


};

struct CdsCreateClass
{
	int IncludeDerived;

	/*!	\brief The allowed create media class of container. See \ref CdsMediaClass for more information.
	*/
	unsigned int MediaClass;

	/*!	\brief Allows \ref CdsCreateClass to be used in a linked list.
	*/
	struct CdsCreateClass *Next;
};

struct CdsSearchClass
{
	int IncludeDerived;

	/*!	\brief The allowed search media class of container. See \ref CdsMediaClass for more information.
	*/
	unsigned int MediaClass;

	/*!	\brief Allows \ref CdsSearchClass to be used in a linked list.
	*/
	struct CdsSearchClass *Next;
};

/*!	\brief Minimalistic representation of a resource.

	All strings must be in UTF-8 form.

	\warning <b>MUSTDO:</b> Need to add support for upnp CR-37 and
	DLNA res properties.
*/
struct CdsResource
{
	/*!	\brief
		Indicates which fields on \ref CdsResource need to be
		deallocated.

	 	If the bit is set, then the corresponding field will be
	 	deleted in a call to \ref CDS_DestroyObject() or
		\ref CDS_DestroyResources().

		See \ref CdsResAllocatable for more information.
	*/
	unsigned int Allocated;

	/*!	\brief Boolean flags for representing boolean properties of a CDS Resource.

		Individual bits can be accessed through \ref CdsResourceProperties.
	 */
	unsigned int Flags;

	/*!	\brief The UTF-8 encoded URI of the &lt;res&gt; element.

		URI must be properly URI-escaped according to
		the rules of the URI's scheme.
		Note that URI-escaping is different than XML-escaped.
	*/
	char *Value;

	/*!	\brief The UTF-8 encoded protocolInfo of the resource.
	*/
	char *ProtocolInfo;

	/*!	\brief The UTF-8 encoded importUri of the resource for Upload.
	*/
	char *ImportUri;

	/*!	\brief The UTF-8 encoded IFO file URI, if needed.
	*/
	char *IfoFileUri;

	/*!	\brief A UTF-8 encoded URI that an uploader can send an IFO file for uploads.
	*/
	char *ImportIfoFileUri;

	/*!	\brief The horizontal resolution. Negative value means value is not set.
	*/
	int ResolutionX;

	/*!	\brief The vertical resolution. Negative value means value is not set.
	*/
	int ResolutionY;

	/*!	\brief The duration, in number of seconds. Negative value means value is not set.
	*/
	int Duration;

	/*!	\brief The bitrate of the resource. If negative, treat as an unset value.
	*/
	int Bitrate;

	/*!	\brief The color depth of the resource. If negative, treat as an unset value.
	*/
	int ColorDepth;

	/*!	\brief The file size of the resource. If negative, treat as an unset value.
	*/
	long Size;

	/*!	\brief The number of bits per sample - should always accompany a valid SampleFrequency. Negative if unknown.
	*/
	int BitsPerSample;

	/*!	\brief The sampling frequency - should always accompany BitsPerSample. Negative if unknown.
	*/
	int SampleFrequency;

	/*!	\brief Number of audio channels. Negative indicates unknown.
	*/
	int NrAudioChannels;

	/*!	\brief App-defined DRM protection string. NULL if not present.
	*/
	char *Protection;

	/*!	\brief Indicates the number of tracks for playlist/media-collection files.

		\note maps to &lt;res\@dlna:trackTotal&gt;
	*/
	int TrackTotal;

	/*!	\brief Indicates if the &lt;res&gt; element supports the
		<i>resume content transfer</i> operation.

		\warning This applies only for upload cases.
	*/
	int ResumeUpload;
	/*!	\brief Indicates if the number of bytes that have been uploaded if the &lt;res&gt; supports the
		<i>resume content transfer</i> operation.  &lt;res\@dlna:resumeUpload&gt; must be "1" if this value is
		greater than 0.

		\warning This applies only for upload cases.
	*/
	int UploadedSize;

	/*!	\brief Allows \ref CdsResource to be used in a linked list.
	*/
	struct CdsResource *Next;
};

/*!	\brief Additional metadata for media objects
	that are or derived from <b>object.item</b>.

	Use a BITWISE-AND operation of
	(\ref CdsObject::MediaClass & \ref CDS_CLASS_MASK_ITEM)
	to determine if \ref CdsObject::TypeObject is
	a \ref CdsObject_Item.
*/
struct CdsObject_Item
{
	/*!	\brief Object ID of underlying item - required only for reference items only

		\warning The length of this field must be less than 256 bytes.
	*/
	char *RefID;
};

/*!	\brief Information used solely for CDS objects in a control point context.
*/
struct CdsObject_ControlPoint
{
	/*!	\brief <b>read-only:</b> UPnP service that provided this object.

		Applications may pass this pointer when operations require
		a pointer to the instance of the UPnP service (for the CDS).
	*/
	struct UPnPService	*ServiceObject;

	/*!	\brief <b>reserved & read-only:</b> used for memory allocation tracking

		\warning Applications generally do not need this information. Applications
		must not modify this property.
	*/
	unsigned long		ReservedMallocSize;

	/*!	\brief <b>reserved & read-only:</b> reference counting.

		\warning Applications generally do not need this information. Applications
		must not modify this property.
	*/
	long				ReservedRefCount;

	/*!	\brief <b>reserved & read-only:</b> semaphore

		\warning Applications generally do not need this information. Applications
		must not modify this property.
	*/
	sem_t				ReservedLock;
};

/*!	\brief Additional metadata for media objects
	that are or derived from <b>object.container</b>.

	Use a BITWISE-AND operation of
	(\ref CdsObject::MediaClass & \ref CDS_CLASS_MASK_CONTAINER)
	to determine if \ref CdsObject::TypeObject is
	a \ref CdsObject_Container.
*/
struct CdsObject_Container
{
	/*!	\brief The number of child CDS object that this container has.

		Negative value means that the number of children was not provided.

		This field should be ignored unless \ref CdsObject::MediaClass
		indicates the CDS object is a CDS container.

		\warning DMS devices need to populate this field when returning
		metadata for a CDS container.
	*/
	int ChildCount;

	/*!	\brief Indicates if &lt;dlna:containerType&gt; is used for a CDS container.

		-A value of 1 indicates that the value
		of <b>Tuner_1_0</b> will be used with the
		&lt;dlna:containerType&gt; metadata property.
		-A value of 0 indicates that &lt;dlna:containerType&gt; is not specified.
		-All other values are undefined at this time.
	*/
	int DlnaContainerType;

	struct CdsCreateClass *CreateClass;

	struct CdsSearchClass *SearchClass;
};

/*!	\brief Additional metadata for media objects
	that are or derived from <b>object.container.album.musicAlbum</b>.

	Use a BITWISE-AND operation of
	(\ref CdsObject::MediaClass & \ref CDS_CLASS_MASK_MINOR1)
	to determine if \ref CdsObject::TypeMinor1 is
	a \ref CdsObject_MusicAlbum.
*/
struct CdsObject_MusicAlbum
{
 	/*!	\brief Indicates the number of entries in
		\ref CdsObject_AudioItem::Genres.
	*/
	unsigned char NumGenres;

	/*!	\brief An array of UTF-8 encoded strings
		that represent the genres associated with the
		CDS object.
	*/
	char **Genres;
};

/*!	\brief Additional metadata for media objects
	that are or derived from <b>object.item.imageItem</b>.

	Use a BITWISE-AND operation of
	(\ref CdsObject::MediaClass & \ref CDS_CLASS_MASK_MAJOR_IMAGEITEM)
	to determine if \ref CdsObject::TypeMajor is
	a \ref CdsObject_ImageItem.
*/
struct CdsObject_ImageItem
{
	/*!	\brief The date when the image was made.

		\note It is stored as UNIX timestamps, which is the number of seconds since January 1, 1970 UTC. 
	*/
	long Date;
};


/*!	\brief Additional metadata for media objects
	that are or derived from <b>object.item.imageItem.photo</b>.

	Use a BITWISE-AND operation of
	(\ref CdsObject::MediaClass & \ref CDS_CLASS_MASK_MINOR1_PHOTO)
	to determine if \ref CdsObject::TypeMinor1 is
	a \ref CdsObject_Photo.
*/
struct CdsObject_Photo
{
	/*!	\brief Indicates the number of entries in
		\ref CdsObject_Photo::Albums.
	*/
	unsigned char NumAlbums;

	/*!	\brief An array of UTF-8 encoded strings
		that represent the albums associated with the
		CDS object.
	*/
	char **Albums;
};


/*!	\brief Additional metadata for media objects
	that are or derived from <b>object.item.videoItem</b>.

	Use a BITWISE-AND operation of
	(\ref CdsObject::MediaClass & \ref CDS_CLASS_MASK_MAJOR_VIDEOITEM)
	to determine if \ref CdsObject::TypeMajor is
	a \ref CdsObject_VideoItem.
*/
struct CdsObject_VideoItem
{
	/*!	\brief The date when the video was made.

		\note It is stored as UNIX timestamps, which is the number of seconds since January 1, 1970 UTC. 
	*/
	long Date;

	/*!	\brief Indicates the number of entries in
		\ref CdsObject_VideoItem::Genres.
	*/
	unsigned char NumGenres;

	/*!	\brief An array of UTF-8 encoded strings
		that represent the genres associated with the
		CDS object.
	*/
	char **Genres;
};

/*!	\brief Additional metadata for media objects
	that are or derived from <b>object.item.videoItem.videoBroadcast</b>.

	Use a BITWISE-AND operation of
	(\ref CdsObject::MediaClass & \ref CDS_CLASS_MASK_MINOR1_VIDEOBROADCAST)
	to determine if \ref CdsObject::TypeMinor1 is
	a \ref CdsObject_VideoBroadcast.
*/
struct CdsObject_VideoBroadcast
{
	/*!	\brief Name of the video broadcast's channel.

		If non-NULL, then it must be a UTF-8 encoded string less than 256 bytes.
	*/
	char *ChannelName;

	/*!	\brief Channel number.

		Use -1 to indicate the value is unassigned.
	*/
	int ChannelNr;
};


/*!	\brief Additional metadata for media objects
	that are or derived from <b>object.item.audioItem</b>.

	Use a BITWISE-AND operation of
	(\ref CdsObject::MediaClass & \ref CDS_CLASS_MASK_MAJOR_AUDIOITEM)
	to determine if \ref CdsObject::TypeMajor is
	a \ref CdsObject_AudioItem.
*/
struct CdsObject_AudioItem
{
	/*!	\brief The date when the audio was made.

		\note It is stored as UNIX timestamps, which is the number of seconds since January 1, 1970 UTC. 
	*/
	long Date;

	/*!	\brief Indicates the number of entries in
		\ref CdsObject_AudioItem::Genres.
	*/
	unsigned char NumGenres;

	/*!	\brief An array of UTF-8 encoded strings
		that represent the genres associated with the
		CDS object.
	*/
	char **Genres;

	/*!	\brief Indicates the number of entries in
		\ref CdsObject_AudioItem::Albums.
	*/
	unsigned char NumAlbums;

	/*!	\brief An array of UTF-8 encoded strings
		that represent the albums associated with the
		CDS object.
	*/
	char **Albums;
};

/*!	\brief Additional metadata for media objects
	that are or derived from <b>object.item.videoItem.audioBroadcast</b>.

	Use a BITWISE-AND operation of
	(\ref CdsObject::MediaClass & \ref CDS_CLASS_MASK_MINOR1_AUDIOBROADCAST)
	to determine if \ref CdsObject::TypeMinor1 is
	a \ref CdsObject_AudioBroadcast.
*/
struct CdsObject_AudioBroadcast
{
	/*!	\brief Name of the audio broadcast's channel.

		If non-NULL, then it must be a UTF-8 encoded string less than 256 bytes.
	*/
	char *ChannelName;

	/*!	\brief Channel number.

		Use -1 to indicate the value is unassigned.
	*/
	int ChannelNr;
};

/*!	\brief Abstracts control-point specific information.
*/
union CpInformation
{
	/*!	\brief Applications that receive CDS objects in a control-point context
		can use this field ot know which UPnP service (i.e. CDS instance)
		created the CDS object.

		If this field is NULL, then it is assumed that the CDS object was not
		created by a control point.
	*/
	struct UPnPService	*ServiceObject;

	/*!	\brief Reserved information.

		Control points that instantiate can use this field to track
		references and memory allocations. The first field of the
		\ref CdsObject_ControlPoint must be a <i>UPnPService*</i>.
	*/
	struct CdsObject_ControlPoint Reserved;
};

/*!	\brief Union for the different base types for the media class of a CDS object.
*/
union ObjectTypes
{
	/*!	\brief Used when the CDS object is or is derived from <b>object.item</b>.
	*/
	struct CdsObject_Item Item;

	/*!	\brief Used when the CDS object is or is derived from <b>object.container</b>.
	*/
	struct CdsObject_Container Container;
};

/*!	\brief Union for different types for the major classifier of a CDS object's class.

	\note This union can be extended to support additional major types.
	To do this, you must do the following.
	- Define a new struct to encapsulate the metadata that applies
	to the major class. The major type should correspond to one of the
	<i>CDS_CLASS_MASK_MAJOR_xxx</i> values.
	- Modify \ref CDS_SerializeObjectToDidl() to support the the new major type.
	- Modify \ref CDS_DeserializeDidlToObject() to support the new major type.
	- Modify \ref CDS_CloneMediaObject() to support the new major type.
*/
union MajorTypes
{
	/*!	\brief Used when the major type is unassigned.
	*/
	char						Unassigned;

	/*!	\brief Used when the class is or is derived from <b>object.item.audioItem</b>
	*/
	struct CdsObject_AudioItem	AudioItem;

	/*!	\brief Used when the class is or is derived from <b>object.item.imageItem</b>
	*/
	struct CdsObject_ImageItem	ImageItem;

	/*!	\brief Used when the class is or is derived from <b>object.item.videoItem</b>
	*/
	struct CdsObject_VideoItem	VideoItem;
};

/*!	\brief Union for the different types for the minor1 classifiers of a CDS object's class.

	\note This union can be extended to support additional minor1 types.
	To do this, you must do the following.
	- Define a new struct to encapsulate the metadata that applies
	to the major class. The minor1 type should correspond to one of the
	<i>CDS_CLASS_MASK_MINOR1_xxx</i> values.
	- Modify \ref CDS_SerializeObjectToDidl() to support the the new minor1 type.
	- Modify \ref CDS_DeserializeDidlToObject() to support the new minor1 type.
	- Modify \ref CDS_CloneMediaObject() to support the new minor1 type.
*/
union Minor1Types
{
	/*!	\brief Used when the minor1 type is unassigned.
	*/
	char						Unassigned;

	/*!	\brief Used when the class is or is derived from <b>object.container.album.musicAlbum</b>
	*/
	struct CdsObject_MusicAlbum	MusicAlbum;

	/*!	\brief Used when the class is or is derived from <b>object.item.imageItem.photo</b>
	*/
	struct CdsObject_Photo	Photo;

	/*!	\brief Used when the class is or is derived from <b>object.item.videoItem.videoBroadcast</b>
	*/
	struct CdsObject_VideoBroadcast	VideoBroadcast;

	/*!	\brief Used when the class is or is derived from <b>object.item.videoItem.audioBroadcast</b>
	*/
	struct CdsObject_AudioBroadcast	AudioBroadcast;
};

/*!	\brief Union for the different types for the minor2 classifiers of a CDS object's class.

	\note This union can be extended to support additional minor2 types.
	To do this, you must do the following.
	- Define a new struct to encapsulate the metadata that applies
	to the major class. The minor2 type should correspond to one of the
	<i>CDS_CLASS_MASK_MINOR2_xxx</i> values.
	- Modify \ref CDS_SerializeObjectToDidl() to support the the new minor2 type.
	- Modify \ref CDS_DeserializeDidlToObject() to support the new minor2 type.
	- Modify \ref CDS_CloneMediaObject() to support the new minor2 type.
*/
union Minor2Types
{
	/*!	\brief Used when the minor1 type is unassigned.
	*/
	char						Unassigned;
};

/*!	\brief Represents the most common properties for a CDS object.
*/
struct CdsObject
{
	/*!	\brief Bit string mapped by \ref CdsAllocatable.

		If the bit is set, then the corresponding field will be
	 	deleted in a call to \ref CDS_DestroyObject().
	 */
	unsigned int DeallocateThese;

	/*!	\brief Object ID - required, must be non-NULL

		\warning The length of this field must be UTF-8 encoded and less than 256 bytes.
	*/
	char *ID;

	/*!	\brief Parent ID - required, must be non-NULL

		\warning The length of this field must be UTF-8 encoded and  less than 256 bytes.
	*/
	char *ParentID;

	/*!	\brief title of the CDS object - required, must be non-NULL

		\warning The length of this field must be UTF-8 encoded and less than 1024 bytes.
	*/
	char *Title;

	/*!	\brief creator/author/artist of the CDS object - optional. Use NULL to leave unassigned.

		\warning The length of this field must be less than 256 bytes.
	*/
	char *Creator;

	/*!	\brief The media class of object. See \ref CdsMediaClass for more information.
	*/
	unsigned int MediaClass;

	/*!	\brief Boolean flags for representing boolean properties of a CDS object.

		Individual bits can be accessed through \ref CdsObjectProperties.
	 */
	unsigned int Flags;


	/*!	\brief Indicates if the CDS object was created for control-point use.

		- If (\ref CpInformation::ServiceObject != NULL) then the CDS object was
		created for a control point use.
	*/
	union CpInformation CpInfo;

	/*!	\brief Boolean flags indicating the bits for the \@dlnaManaged attribute.

		Individual bits can be access through \ref CdsDlnaManagedBits.
	*/
	unsigned int DlnaManaged;

	/*!	\brief Indicates the &lt;dlna:takeOut&gt; group names for the CDS object.

		\warning Each string must be UTF-8 encoded and 256 bytes (i.e. defined by \b CDS_MAX_TAKEOUT_GROUPNAME_LEN) or less.
	*/
	char **TakeOutGroups;

	/*!	\brief Indicates the number of strings in \ref CdsObject::TakeOutGroups.
	*/
	int NumTakeOutGroups;

	/*!	\brief Additional metadata for this object, based on whether it is a container or item.

		A BITWISE-AND operation of
		(\ref CdsObject::MediaClass & \ref CDS_CLASS_MASK_OBJECT_TYPE)
		will indicate the type of data that is available
		for this union.
		- If the result is \ref CDS_CLASS_MASK_CONTAINER
		then it is a \b container, with additional metadata
		found at \ref ObjectTypes::Container.
		- If the result is \ref CDS_CLASS_MASK_ITEM
		then it is an \b item, with additional metadata
		found at \ref ObjectTypes::Item.
	*/
	union ObjectTypes TypeObject;

	/*!	\brief Additional metadata for this object, based on the MAJOR classifier for the
		CDS object's class.

		A BITWISE-AND operation of
		(\ref CdsObject::MediaClass & \ref CDS_CLASS_MASK_MAJOR)
		will indicate the type of data that is available
		for this union.
		- If the result is \ref CDS_CLASS_MASK_MAJOR_AUDIOITEM
		then it is an \b audioItem, with additional metadata
		found at \ref MajorTypes::AudioItem.
		- If the result is \ref CDS_CLASS_MASK_MAJOR_IMAGEITEM
		then it is an \b imageItem, with additional metadata
		found at \ref MajorTypes::ImageItem.
		- If the result is \ref CDS_CLASS_MASK_MAJOR_AUDIOITEM
		then it is an \b audioItem, with additional metadata
		found at \ref MajorTypes::AudioItem.
	*/
	union MajorTypes TypeMajor;

	/*!	\brief Additional metadata for this object, based on the MINOR1 classifier for the
		CDS object's class.

		A BITWISE-AND operation of
		(\ref CdsObject::MediaClass & \ref CDS_CLASS_MASK_MINOR1)
		will indicate the type of data that is available
		for this union.
		- If the result is \ref CDS_CLASS_MASK_MINOR1_MUSICALBUM
		then it is a \b musicAlbum container, with additional metadata
		found at \ref Minor1Types::MusicAlbum.
		- If the result is \ref CDS_CLASS_MASK_MINOR1_PHOTO
		then it is a \b photo item, with additional metadata
		found at \ref Minor1Types::Photo.
		- If the result is \ref CDS_CLASS_MASK_MINOR1_VIDEOBROADCAST
		then it is a \b videoBroadcast item, with additional metadata
		found at \ref Minor1Types::VideoBroadcast.
		- If the result is \ref CDS_CLASS_MASK_MINOR1_AUDIOBROADCAST
		then it is a \b audioBroadcast item, with additional metadata
		found at \ref Minor1Types::AudioBroadcast.
	*/
	union Minor1Types TypeMinor1;


	/*!	\brief Additional metadata for this object, based on the MINOR1 classifier for the
		CDS object's class.
	*/
	union Minor2Types TypeMinor2;

	/*!	\brief First resource for the CDS object
	*/
	struct CdsResource *Res;
	char *Source;

	/*!	\brief for Upload use;
	*/
	void* User;
};

/*!	\brief Allocates the memory for a \ref CdsObject, with all memory set to zero.
	\returns A blank \ref CdsObject.
*/
struct CdsObject* CDS_AllocateObject();

/*!	\brief Allocates the memory for a \ref CdsResource, with all memory set to zero.
	\returns A blank \ref CdsResource.
*/
struct CdsResource* CDS_AllocateResource();

/*!	\brief <b>DEPRECATED</b> Deallocates all of the memory for the linked list of \ref CdsObject
	instances and their associated resources.

	\param[in] cds_obj Linked list of \ref CdsObject instances.
	Use <i>CdsObject::Next</i> ==NULL to terminate the linked list.

	\warning Do not call this method directly to destroy a created CdsObject using CDS_AllocateObject().
	Use \ref CDS_ObjRef_Release() instead.
*/
void CDS_DestroyObject(struct CdsObject *cds_obj);

/*!	\brief Deallocates all of the memory for the linked list of \ref CdsResource
	instances.

	\param[in] res_list Linked list of \ref CdsResource instances.
	Use \ref CdsResource::Next ==NULL to terminate the linked list.
*/
void CDS_DestroyResources(struct CdsResource *res_list);

/*! \} */


#endif

