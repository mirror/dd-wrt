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
 * $Workfile: CdsMediaClass.h
 *
 *
 *
 */

#ifndef _CDS_MEDIACLASS_H
#define _CDS_MEDIACLASS_H

/* NOTDONE: Need to add support for DLNA-derived classes, object.container.tunerContainer. */

/*! \file CdsMediaClass.h 
	\brief Reduces the memory representation of upnp:class values.
*/

/*! \defgroup CdsMediaClass CDS Helper - Media Classes
	\brief Reduces the memory representation of upnp:class values.

 	The purpose of this module is to abstract the concept of a upnp media class,
 	(eg, the data stored in the &lt;upnp:class&gt; DIDL-Lite element) into 
	an unsigned integer form (of at least 16 bits).

 	The module also provides the means to allow vendors to add their own custom
 	media classes to the existing infrastructure. 

 	A media class is represented in string form for UPnP, but can be exposed to applications
 	through an unsigned int of 16 bits. If the value of the media class is CDS_CLASS_MASK_BADCLASS (zero), 
 	then it should mean that the media class is unassigned or invalid in some form.
 
 	For all intents and purposes, if a bitwise-AND operation with a \a CDS_CLASS_MASK_xxx value yields
 	a value of zero, then it means that the particulate portion of the media class remains unassigned.

	Described below is a description of how the bits are used in the value.
 	Please note that the bit-string is organized such that sorting the 16-bit values
 	will correspond appropriately with the sorting order of the same classes in 
 	in string form.

	- <b>bits [0,5]</b>
	6 bits to represent customized extensions.
 	If vendors need more extensions, then a 32-bit value can be used.
 	Extract these bits out using a bitwise-AND operation of media class & 
	\ref CDS_CLASS_MASK_MINOR2,
 	shift right the resulting value by \ref CDS_SHIFT_MINOR2_TYPE bits,
 	and use the value as an index into the as \ref CDS_CLASS_MINOR2_TYPE array.

 	- <b>bits [6,9]</b>
	4 bits to represent minor classes.
 	There are 12 normative values for UPnP AV CDS:
 	Extract these bits out using a bitwise-AND operation of media class & \a CDS_CLASS_MASK_MINOR1_xxx,
 	shift right the resulting value by \ref CDS_SHIFT_MINOR1_TYPE bits,
	and use the value as an index into the as \ref CDS_CLASS_MINOR1_TYPE array.
		- musicAlbum
		- photoAlbum
		- musicGenre
		- movieGenre
 		- musicArtist
		- audioBook
		- audioBroadcast
		- musicTrack
		- photo
		- movie
		- musicVideoClip
		- videoBroadcast

	- <b>bits [10,13]</b>
 	4 bits to represent major classes.
 	There are 12 normative values for UPnP AV CDS:
 	Extract these bits out using a bitwise-AND operation of media class & \a CDS_CLASS_MASK_MAJOR_xxx,
 	shift right the resulting value by \ref CDS_SHIFT_MAJOR_TYPE bits,
 	and use the value as an index into the as CDS_CLASS_MAJOR_TYPE array.
		- album
		- genre
		- person
		- playlistContainer, 
 		- storageFolder
		- storageSystem
		- storageVolume
		- audioItem
		- imageItem, 
 		- playlistItem
		- textItem
		- videoItem

 	- <b>bit 14</b>
	Flag: object is container. 
	Extract this bit out using a bitwise-AND operation of media class & \a CDS_CLASS_MASK_OBJECT_TYPE_xxx,
 	shift right the resulting value by \ref CDS_SHIFT_OBJECT_TYPE bits,
 	and use the value as an index into the as \ref CDS_CLASS_OBJECT_TYPE array.

	- <b>bit 15</b>
	Flag: object is item. 
	Extract this bit out using a bitwise-AND operation of media class & \a CDS_CLASS_MASK_OBJECT_TYPE_xxx,
 	shift right the resulting value by \ref CDS_SHIFT_OBJECT_TYPE bits,
 	and use the value as an index into the as \ref CDS_CLASS_OBJECT_TYPE array.

 	To obtain a string representation of a media class, concatenate the strings that represent the
 	object-type, major-type, minor type, and minor2 types with each substring separated with a 
 	a period (.) character. There is no trailing dot character. For example, the
 	media class of 0xA200 (which has binary form: 10 1000 1000 000000) would have a 
 	corresponding string representation of <i>object.item.audioItem.musicTrack</i> because:
 	- CDS_CLASS_OBJECT_TYPE[(0xA200 & CDS_CLASS_MASK_OBJECTTYPE) >> CDS_SHIFT_OBJECT_TYPE] == "object.item"
 	- CDS_CLASS_MAJOR_TYPE[(0xA200 & CDS_CLASS_MASK_MAJOR) >> CDS_SHIFT_MAJOR_TYPE] == "audioItem"
 	- CDS_CLASS_MINOR1_TYPE[(0xA200 & CDS_CLASS_MASK_MAJOR) >> CDS_SHIFT_MINOR1_TYPE] == "musicTrack"

	Another added benefit of this model is that it's friendly to boolean operations that involve
 	the media class. For example, the following are common operations.
  		- <b>((mediaClass & CDS_CLASS_MASK_ITEM))</b>:
		Indicates if the media class is some kind of item
 		- <b>((mediaClass & CDS_CLASS_MASK_CONTAINER))</b>:
 		Indicates if the media class is some kind of container
		- <b>((mediaClass & CDS_CLASS_MASK_MAJOR_AUDIOITEM)==CDS_CLASS_MASK_MAJOR_AUDIOITEM)</b>:
        Indicates if the media class is some kind of audio related item

 	The header file also defines a set of \a CDS_CLASS_MASK_xxx values to match up with the 
 	normative set of media classes defined in UPnP AV.
 
 	To obtain a value for a normative media class, use one of the \a CDS_MEDIACLASS_xxx values. 
 	For example, to get the media class value for an audio broadcast, use \ref CDS_MEDIACLASS_AUDIOBROADCAST.
 	Using \ref CDS_CLASS_MASK_MINOR1_AUDIOBROADCAST will only set the minor1 portion of the media class,
 	which is an incomplete media class. 
	\{
*/

/*!	\brief Static array of the normative object types: eg, object.item or object.container 
	For example: <b>object.item</b>, <b>object.container</b>.
*/
extern const char* CDS_CLASS_OBJECT_TYPE[];	

/*!	\brief Static array of the normative major classes (the first sublevel from item or container). 
	For example: <b>audioItem</b>, <b>album</b>.
*/
extern const char* CDS_CLASS_MAJOR_TYPE[];	

/*! \brief Static array of the normative set of minor class designations (the second sublevel from item or container).
	For example: <b>musicTrack</b>, <b>musicAlbum</b>.
*/
extern const char* CDS_CLASS_MINOR1_TYPE[];	

/*! \brief Static array of the customizable minor class designations (the third sublevel from item or container).
*/
extern const char* CDS_CLASS_MINOR2_TYPE[];	

/*!	\brief The number of strings in \ref CDS_CLASS_OBJECT_TYPE.
*/
#define CDS_CLASS_OBJECT_TYPE_LEN	3

/*!	\brief The number of strings in \ref CDS_CLASS_MAJOR_TYPE.
*/
#define CDS_CLASS_MAJOR_TYPE_LEN	13 

/*!	\brief The number of strings in \ref CDS_CLASS_MINOR1_TYPE.
*/
#define CDS_CLASS_MINOR1_TYPE_LEN	13

/*	\brief The number of string in \ref CDS_CLASS_MINOR2_TYPE.
*/
#define CDS_CLASS_MINOR2_TYPE_LEN	1

/*!	\brief The number of bits to shift-right to acquire the object type.
	
	MINOR2 + MINOR1 + MAJOR = 6 + 4 + 4 = 14
*/
#define CDS_SHIFT_OBJECT_TYPE					14

/*!	\brief The number of bits to shift-right to acquire the major type.

	MINOR2 + MINOR1 = 6 + 4 = 10
*/
#define CDS_SHIFT_MAJOR_TYPE					10

/*! \brief The number of bits to shift-right to acquire the minor type.
	
	MINOR2 = 6 
*/
#define CDS_SHIFT_MINOR1_TYPE					6

/*! \brief The number of bits to shift-right to acquire the minor2/custom types.
*/
#define CDS_SHIFT_MINOR2_TYPE					0

/*!	\brief The bit-mask where the object type occupies a 16-bit string.

	Binary value is <i>11 0000 0000 000000</i>.
*/
#define CDS_CLASS_MASK_OBJECT_TYPE				0xC000	

/*!	\brief The bit-mask where the major type occupies a 16-bit string.

	Binary value is <i>00 1111 0000 000000</i>.
*/
#define CDS_CLASS_MASK_MAJOR					0x3C00	

/*!	\brief The bit-mask where the minor1 type occupies a 16-bit string.

	Binary value is <i>00 0000 1111 000000</i>.
*/
#define CDS_CLASS_MASK_MINOR1					0x03C0	

/*!	\brief The bit-mask where the minor2/custom type occupies a 16-bit string.

	Binary value is <i>00 0000 0000 111111</i>.
*/
#define CDS_CLASS_MASK_MINOR2					0x003F

/*!	\brief Use this to indicate an unassigned media class value.
	Usually used for initialization.
*/
#define CDS_CLASS_MASK_BADCLASS					0x0000

/*!	\brief Use this to set or check a media class against <b>object.container</b>
	in the object type portion of the media class.
	
	Binary value: <i>01 0000 0000 000000</i>
*/
#define CDS_CLASS_MASK_CONTAINER				0x4000	

/*!	\brief Use this to set or check a media class against <b>object.item</b>
	in the object type portion of the media class.
	
	Binary value: <i>10 0000 0000 000000</i>
*/
#define CDS_CLASS_MASK_ITEM						0x8000

/*!	\brief Use this to set or check a media class against <b>album</b>
	in the major type portion of the media class.
	
	Binary value: <i>00 0001 0000 000000</i>
*/
#define CDS_CLASS_MASK_MAJOR_ALBUM				0x0400	

/*!	\brief Use this to set or check a media class against <b>genre</b>
	in the major type portion of the media class.
	
	Binary value: <i>00 0010 0000 000000</i>
*/
#define CDS_CLASS_MASK_MAJOR_GENRE				0x0800	

/*!	\brief Use this to set or check a media class against <b>person</b>
	in the major type portion of the media class.
	
	Binary value: <i>00 0011 0000 000000</i>
*/
#define CDS_CLASS_MASK_MAJOR_PERSON				0x0C00	

/*!	\brief Use this to set or check a media class against <b>playlistContainer</b>
	in the major type portion of the media class.
	
	Binary value: <i>00 0100 0000 000000</i>
*/
#define CDS_CLASS_MASK_MAJOR_PLAYLISTCONTAINER	0x1000	

/*!	\brief Use this to set or check a media class against <b>storageFolder</b>
	in the major type portion of the media class.
	
	Binary value: <i>00 0101 0000 000000</i>
*/
#define CDS_CLASS_MASK_MAJOR_STORAGEFOLDER		0x1400

/*!	\brief Use this to set or check a media class against <b>storageSystem</b>
	in the major type portion of the media class.
	
	Binary value: <i>00 0110 0000 000000</i>
*/
#define CDS_CLASS_MASK_MAJOR_STORAGESYSTEM		0x1800

/*!	\brief Use this to set or check a media class against <b>storageVolume</b>
	in the major type portion of the media class.
	
	Binary value: <i>00 0111 0000 000000</i>
*/
#define CDS_CLASS_MASK_MAJOR_STORAGEVOLUME		0x1C00


/*!	\brief Use this to set or check a media class against <b>audioItem</b>
	in the major type portion of the media class.
	
	Binary value: <i>00 1000 0000 000000</i>
*/
#define CDS_CLASS_MASK_MAJOR_AUDIOITEM			0x2000

/*!	\brief Use this to set or check a media class against <b>imageItem</b>
	in the major type portion of the media class.
	
	Binary value: <i>00 1001 0000 000000</i>
*/
#define CDS_CLASS_MASK_MAJOR_IMAGEITEM			0x2400

/*!	\brief Use this to set or check a media class against <b>playlistItem</b>
	in the major type portion of the media class.
	
	Binary value: <i>00 1010 0000 000000</i>
*/
#define CDS_CLASS_MASK_MAJOR_PLAYLISTITEM		0x2800

/*!	\brief Use this to set or check a media class against <b>textItem</b>
	in the major type portion of the media class.
	
	Binary value: <i>00 1011 0000 000000</i>
*/
#define CDS_CLASS_MASK_MAJOR_TEXTITEM			0x2C00

/*!	\brief Use this to set or check a media class against <b>videoItem</b>
	in the major type portion of the media class.
	
	Binary value: <i>00 1100 0000 000000</i>
*/
#define CDS_CLASS_MASK_MAJOR_VIDEOITEM			0x3000

/*!	\brief Use this to set or check a media class against <b>musicAlbum</b>
	in the minor1 type portion of the media class.
	
	Binary value: <i>00 0000 0001 000000</i>
*/
#define CDS_CLASS_MASK_MINOR1_MUSICALBUM		0x0040

/*!	\brief Use this to set or check a media class against <b>photoAlbum</b>
	in the minor1 type portion of the media class.
	
	Binary value: <i>00 0000 0010 000000</i>
*/
#define CDS_CLASS_MASK_MINOR1_PHOTOALBUM		0x0080	

/*!	\brief Use this to set or check a media class against <b>movieGenre</b>
	in the minor1 type portion of the media class.
	
	Binary value: <i>00 0000 0011 000000</i>
*/
#define CDS_CLASS_MASK_MINOR1_MOVIEGENRE		0x00C0	

/*!	\brief Use this to set or check a media class against <b>musicGenre</b>
	in the minor1 type portion of the media class.
	
	Binary value: <i>00 0000 0100 000000</i>
*/
#define CDS_CLASS_MASK_MINOR1_MUSICGENRE		0x0100

/*!	\brief Use this to set or check a media class against <b>musicArtist</b>
	in the minor1 type portion of the media class.
	
	Binary value: <i>00 0000 0101 000000</i>
*/
#define CDS_CLASS_MASK_MINOR1_MUSICARTIST		0x0140

/*!	\brief Use this to set or check a media class against <b>audioBook</b>
	in the minor1 type portion of the media class.
	
	Binary value: <i>00 0000 0110 000000</i>
*/
#define CDS_CLASS_MASK_MINOR1_AUDIOBOOK			0x0180	

/*!	\brief Use this to set or check a media class against <b>audioBroadcast</b>
	in the minor1 type portion of the media class.
	
	Binary value: <i>00 0000 0111 000000</i>
*/
#define CDS_CLASS_MASK_MINOR1_AUDIOBROADCAST	0x01C0	

/*!	\brief Use this to set or check a media class against <b>musicTrack</b>
	in the minor1 type portion of the media class.
	
	Binary value: <i>00 0000 1000 000000</i>
*/
#define CDS_CLASS_MASK_MINOR1_MUSICTRACK		0x0200

/*!	\brief Use this to set or check a media class against <b>photo</b>
	in the minor1 type portion of the media class.
	
	Binary value: <i>00 0000 1001 000000</i>
*/
#define CDS_CLASS_MASK_MINOR1_PHOTO				0x0240	

/*!	\brief Use this to set or check a media class against <b>movie</b>
	in the minor1 type portion of the media class.
	
	Binary value: <i>00 0000 1010 000000</i>
*/
#define CDS_CLASS_MASK_MINOR1_MOVIE				0x0280	

/*!	\brief Use this to set or check a media class against <b>musicVideoClip</b>
	in the minor1 type portion of the media class.
	
	Binary value: <i>00 0000 1011 000000</i>
*/
#define CDS_CLASS_MASK_MINOR1_MUSICVIDEOCLIP	0x02C0	

/*!	\brief Use this to set or check a media class against <b>videoBroadcast</b>
	in the minor1 type portion of the media class.
	
	Binary value: <i>00 0000 1100 000000</i>
*/
#define CDS_CLASS_MASK_MINOR1_VIDEOBROADCAST	0x0300	

/*!	\brief Set a media class to this value for <b>object.item</b>.
*/
#define CDS_MEDIACLASS_ITEM						(CDS_CLASS_MASK_ITEM)

/*!	\brief Set a media class to this value for <b>object.container</b>.
*/
#define CDS_MEDIACLASS_CONTAINER				(CDS_CLASS_MASK_CONTAINER)

/*!	\brief Set a media class to this value for <b>object.item.imageItem</b>.
*/
#define CDS_MEDIACLASS_IMAGEITEM				(CDS_CLASS_MASK_ITEM | CDS_CLASS_MASK_MAJOR_IMAGEITEM			)

/*!	\brief Set a media class to this value for <b>object.item.audioItem</b>.
*/
#define CDS_MEDIACLASS_AUDIOITEM				(CDS_CLASS_MASK_ITEM | CDS_CLASS_MASK_MAJOR_AUDIOITEM			)

/*!	\brief Set a media class to this value for <b>object.item.videoItem</b>.
*/
#define CDS_MEDIACLASS_VIDEOITEM				(CDS_CLASS_MASK_ITEM | CDS_CLASS_MASK_MAJOR_VIDEOITEM			)

/*!	\brief Set a media class to this value for <b>object.item.playlistItem</b>.
*/
#define CDS_MEDIACLASS_PLAYLISTITEM				(CDS_CLASS_MASK_ITEM | CDS_CLASS_MASK_MAJOR_PLAYLISTITEM		)

/*!	\brief Set a media class to this value for <b>object.item.textItem</b>.
*/
#define CDS_MEDIACLASS_TEXTITEM					(CDS_CLASS_MASK_ITEM | CDS_CLASS_MASK_MAJOR_TEXTITEM			)

/*!	\brief Set a media class to this value for <b>object.container.person</b>.
*/
#define CDS_MEDIACLASS_PERSON					(CDS_CLASS_MASK_CONTAINER | CDS_CLASS_MASK_MAJOR_PERSON				)

/*!	\brief Set a media class to this value for <b>object.container.playlistContainer</b>.
*/
#define CDS_MEDIACLASS_PLAYLISTCONTAINER		(CDS_CLASS_MASK_CONTAINER | CDS_CLASS_MASK_MAJOR_PLAYLISTCONTAINER	)

/*!	\brief Set a media class to this value for <b>object.container.album</b>.
*/
#define CDS_MEDIACLASS_ALBUM					(CDS_CLASS_MASK_CONTAINER | CDS_CLASS_MASK_MAJOR_ALBUM				)

/*!	\brief Set a media class to this value for <b>object.container.genre</b>.
*/
#define CDS_MEDIACLASS_GENRE					(CDS_CLASS_MASK_CONTAINER | CDS_CLASS_MASK_MAJOR_GENRE				)

/*!	\brief Set a media class to this value for <b>object.container.storageSystem</b>.
*/
#define CDS_MEDIACLASS_STORAGESYSTEM			(CDS_CLASS_MASK_CONTAINER | CDS_CLASS_MASK_MAJOR_STORAGESYSTEM		)

/*!	\brief Set a media class to this value for <b>object.container.storageVolume</b>.
*/
#define CDS_MEDIACLASS_STORAGEVOLUME			(CDS_CLASS_MASK_CONTAINER | CDS_CLASS_MASK_MAJOR_STORAGEVOLUME		)

/*!	\brief Set a media class to this value for <b>object.container.storageFolder</b>.
*/
#define CDS_MEDIACLASS_STORAGEFOLDER			(CDS_CLASS_MASK_CONTAINER | CDS_CLASS_MASK_MAJOR_STORAGEFOLDER		)

/*!	\brief Set a media class to this value for <b>object.item.imageItem.photo</b>.
*/
#define CDS_MEDIACLASS_PHOTO					(CDS_CLASS_MASK_ITEM | CDS_CLASS_MASK_MAJOR_IMAGEITEM | CDS_CLASS_MASK_MINOR1_PHOTO			)

/*!	\brief Set a media class to this value for <b>object.item.audioItem.musicTrack</b>.
*/
#define CDS_MEDIACLASS_MUSICTRACK				(CDS_CLASS_MASK_ITEM | CDS_CLASS_MASK_MAJOR_AUDIOITEM | CDS_CLASS_MASK_MINOR1_MUSICTRACK	)

/*!	\brief Set a media class to this value for <b>object.item.audioItem.audioBroadcast</b>.
*/
#define CDS_MEDIACLASS_AUDIOBROADCAST			(CDS_CLASS_MASK_ITEM | CDS_CLASS_MASK_MAJOR_AUDIOITEM | CDS_CLASS_MASK_MINOR1_AUDIOBROADCAST)

/*!	\brief Set a media class to this value for <b>object.item.audioItem.audioBook</b>.
*/
#define CDS_MEDIACLASS_AUDIOBOOK				(CDS_CLASS_MASK_ITEM | CDS_CLASS_MASK_MAJOR_AUDIOITEM | CDS_CLASS_MASK_MINOR1_AUDIOBOOK		)

/*!	\brief Set a media class to this value for <b>object.item.videoItem.movie</b>.
*/
#define CDS_MEDIACLASS_MOVIE					(CDS_CLASS_MASK_ITEM | CDS_CLASS_MASK_MAJOR_VIDEOITEM | CDS_CLASS_MASK_MINOR1_MOVIE			)

/*!	\brief Set a media class to this value for <b>object.item.videoItem.videoBroadcast</b>.
*/
#define CDS_MEDIACLASS_VIDEOBROADCAST			(CDS_CLASS_MASK_ITEM | CDS_CLASS_MASK_MAJOR_VIDEOITEM | CDS_CLASS_MASK_MINOR1_VIDEOBROADCAST)

/*!	\brief Set a media class to this value for <b>object.item.videoItem.musicVideoClip</b>.
*/
#define CDS_MEDIACLASS_MUSICVIDEOCLIP			(CDS_CLASS_MASK_ITEM | CDS_CLASS_MASK_MAJOR_VIDEOITEM | CDS_CLASS_MASK_MINOR1_MUSICVIDEOCLIP)

/*!	\brief Set a media class to this value for <b>object.container.person.musicArtists</b>.
*/
#define CDS_MEDIACLASS_MUSICARTIST				(CDS_CLASS_MASK_CONTAINER | CDS_CLASS_MASK_MAJOR_PERSON | CDS_CLASS_MASK_MINOR1_MUSICARTIST	)

/*!	\brief Set a media class to this value for <b>object.container.album.musicAlbum</b>.
*/
#define CDS_MEDIACLASS_MUSICALBUM				(CDS_CLASS_MASK_CONTAINER | CDS_CLASS_MASK_MAJOR_ALBUM  | CDS_CLASS_MASK_MINOR1_MUSICALBUM	)

/*!	\brief Set a media class to this value for <b>object.container.album.photoAlbum</b>.
*/
#define CDS_MEDIACLASS_PHOTOALBUM				(CDS_CLASS_MASK_CONTAINER | CDS_CLASS_MASK_MAJOR_ALBUM  | CDS_CLASS_MASK_MINOR1_PHOTOALBUM	)

/*!	\brief Set a media class to this value for <b>object.container.genre.musicGenre</b>.
*/
#define CDS_MEDIACLASS_MUSICGENRE				(CDS_CLASS_MASK_CONTAINER | CDS_CLASS_MASK_MAJOR_GENRE  | CDS_CLASS_MASK_MINOR1_MUSICGENRE	)

/*!	\brief Set a media class to this value for <b>object.container.genre.movieGenre</b>.
*/
#define CDS_MEDIACLASS_MOVIEGENRE				(CDS_CLASS_MASK_CONTAINER | CDS_CLASS_MASK_MAJOR_GENRE  | CDS_CLASS_MASK_MINOR1_MOVIEGENRE	)

/*! \} */

#endif

