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
 * $Workfile: CdsString.h
 *
 *
 *
 */

#ifndef _CDS_STRINGS_H
#define _CDS_STRINGS_H

/*
 *	Number of chars needed to represent a 32 bit integer, including sign and null char.
 */
#define SIZE_INT32_AS_CHARS						12

/*
 *	Defines a number of well-defined CDS related strings
 *	for UPnP argument names.
 */

#define CDS_STRING_URN_CDS						"urn:schemas-upnp-org:service:ContentDirectory:1"
#define CDS_STRING_BROWSE						"Browse"
#define CDS_STRING_BROWSEMETADATA				"BrowseMetadata"
#define CDS_STRING_BROWSEDIRECTCHILDREN			"BrowseDirectChildren"
#define CDS_STRING_SEARCH						"Search"
#define CDS_STRING_RESULT						"Result"
#define CDS_STRING_NUMBER_RETURNED				"NumberReturned"
#define CDS_STRING_TOTAL_MATCHES				"TotalMatches"
#define CDS_STRING_UPDATE_ID					"UpdateID"
#define CDS_STRING_CREATE_OBJECT				"CreateObject"
#define CDS_STRING_DESTROY_OBJECT				"DestroyObject"
#define CDS_STRING_BROWSE_DIRECT_CHILDREN		"BrowseDirectChildren"
#define CDS_STRING_BROWSE_METADATA				"BrowseMetadata"

#define CDS_STRING_CHILDCOUNT					"@childCount"
#define CDS_STRING_CHILDCOUNT_LEN				11

#define CDS_STRING_SEARCHABLE					"@searchable"
#define CDS_STRING_SEARCHABLE_LEN				11

#define CDS_STRING_DLNAMANAGED					"@dlna:dlnaManaged"
#define CDS_STRING_DLNAMANAGED_LEN				17

#define CDS_STRING_CONTAINER_CHILDCOUNT			"container@childCount"
#define CDS_STRING_CONTAINER_CHILDCOUNT_LEN		20

#define CDS_STRING_CONTAINER_SEARCHABLE			"container@searchable"
#define CDS_STRING_CONTAINER_SEARCHABLE_LEN		20

#define CDS_STRING_TITLE						"dc:title"
#define CDS_STRING_TITLE_LEN					8

#define CDS_STRING_CREATOR						"dc:creator"
#define CDS_STRING_CREATOR_LEN					10

#define CDS_STRING_DATE							"dc:date"
#define CDS_STRING_DATE_LEN						7

#define CDS_STRING_ALBUM						"upnp:album"
#define CDS_STRING_ALBUM_LEN					10

#define CDS_STRING_CLASS						"upnp:class"
#define CDS_STRING_CLASS_LEN					10

#define CDS_STRING_GENRE						"upnp:genre"
#define CDS_STRING_GENRE_LEN					10

#define CDS_STRING_RES							"res"
#define CDS_STRING_RES_LEN						3

#define CDS_STRING_RES_BITRATE					"res@bitrate"
#define CDS_STRING_RES_BITRATE_LEN				11

#define CDS_STRING_RES_BITSPERSAMPLE			"res@bitsPerSample"
#define CDS_STRING_RES_BITSPERSAMPLE_LEN		17

#define CDS_STRING_RES_COLORDEPTH				"res@colorDepth"
#define CDS_STRING_RES_COLORDEPTH_LEN			14

#define CDS_STRING_RES_DURATION					"res@duration"
#define CDS_STRING_RES_DURATION_LEN				12

#define CDS_STRING_RES_PROTECTION				"res@protection"
#define CDS_STRING_RES_PROTECTION_LEN			14

#define CDS_STRING_RES_RESOLUTION				"res@resolution"
#define CDS_STRING_RES_RESOLUTION_LEN			14

#define CDS_STRING_RES_SAMPLEFREQUENCY			"res@sampleFrequency"
#define CDS_STRING_RES_SAMPLEFREQUENCY_LEN		19

#define CDS_STRING_RES_NRAUDIOCHANNELS			"res@nrAudioChannels"
#define CDS_STRING_RES_NRAUDIOCHANNELS_LEN		19

#define CDS_STRING_RES_SIZE						"res@size"
#define CDS_STRING_RES_SIZE_LEN					8

#define CDS_STRING_RES_IMPORTURI				"res@importUri"
#define CDS_STRING_RES_IMPORTURI_LEN			13

#define CDS_STRING_RES_IFOFILEURI				"res@dlna:ifoFileURI"
#define CDS_STRING_RES_IFOFILEURI_LEN			19

#define CDS_STRING_RES_IMPORTIFOFILEURI			"res@importIfoFileUri"
#define CDS_STRING_RES_IMPORTIFOFILEURI_LEN		20

#define CDS_STRING_RES_RESUMEUPLOAD				"res@dlna:resumeUpload"
#define CDS_STRING_RES_RESUMEUPLOAD_LEN			21

#define CDS_STRING_RES_UPLOADEDSIZE				"res@dlna:uploadedSize"
#define CDS_STRING_RES_UPLOADEDSIZE_LEN			21

#define CDS_STRING_RES_TRACKTOTAL				"res@dlna:trackTotal"
#define CDS_STRING_RES_TRACKTOTAL_LEN			19

#define CDS_STRING_ALLIP						"ALLIP"
#define CDS_STRING_ALLIP_LEN					5

#define CDS_STRING_ALL_FILTER					"@dlna:dlnaManaged,dc:title,dc:creator,upnp:class,dc:date,upnp:album,upnp:genre,res,res@bitrate,res@bitsPerSample,res@colorDepth,res@duration,res@protection,res@resolution,res@sampleFrequency,res@nrAudioChannels,res@size,res@importUri,res@dlna:ifoFileURI,res@importIfoFileUri,res@dlna:resumeUpload,res@dlna:uploadedSize,res@dlna:trackTotal"
#define CDS_STRING_ALL_FILTER_STAR				"*"

#define CDS_STRING_HTTP_PROTINFO				"http-get"
#define CDS_STRING_HTTP_PROTINFO_LEN			8

#define CDS_STRING_HTTP_SCHEME					"http://"
#define CDS_STRING_HTTP_SCHEME_LEN				7

#define CDS_STRING_PLAYSINGLE_ITEM_PREFIX		"playsingle-"
#define CDS_STRING_PLAYSINGLE_ITEM_PREFIX_LEN	11
#define CDS_STRING_PLAYSINGLE_SCHEME			"dlna-playsingle://"
#define CDS_STRING_PLAYSINGLE_SCHEME_LEN		18
#define CDS_STRING_PLAYSINGLE_SERVICEID_CDS		"serviceId=ContentDirectory"
#define CDS_STRING_PLAYSINGLE_SERVICEID_CDS_LEN	26
#define CDS_STRING_PLAYSINGLE_ITEMID			"&ItemID="
#define CDS_STRING_PLAYSINGLE_ITEMID_LEN		8

#define CDS_STRING_DLNA_ANYCONTAINER			"DLNA.ORG_AnyContainer"
#define CDS_STRING_DLNA_ANYCONTAINER_LEN		21

#define CDS_MAX_TAKEOUT_GROUPNAME_LEN			256
#define CDS_DATE_VALUE_LEN						20
#define CDS_NUMBER_VALUE_LEN					22

/*
 *	Defines a number of well-defined CDS related strings
 *	for serializing CDS object to DIDL-Lite.
 */

#define CDS_DIDL_HEADER_ESCAPED									"&lt;DIDL-Lite xmlns:dc=&quot;http://purl.org/dc/elements/1.1/&quot; xmlns:upnp=&quot;urn:schemas-upnp-org:metadata-1-0/upnp/&quot; xmlns=&quot;urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/&quot; xmlns:dlna=&quot;urn:schemas-dlna-org:metadata-1-0/&quot;&gt;"
#define CDS_DIDL_HEADER_ESCAPED_LEN								255

#define CDS_DIDL_FOOTER_ESCAPED									"\r\n&lt;/DIDL-Lite&gt;"
#define CDS_DIDL_FOOTER_ESCAPED_LEN								20

#define CDS_DIDL_TITLE1_ESCAPED									"\r\n&lt;dc:title&gt;"
#define CDS_DIDL_TITLE2_ESCAPED									"&lt;/dc:title&gt;"
#define CDS_DIDL_TITLE_ESCAPED_LEN								35

#define CDS_DIDL_CREATOR1_ESCAPED								"\r\n&lt;dc:creator&gt;"
#define CDS_DIDL_CREATOR2_ESCAPED								"&lt;/dc:creator&gt;"
#define CDS_DIDL_CREATOR_ESCAPED_LEN							39

#define CDS_DIDL_ALBUM1_ESCAPED									"\r\n&lt;upnp:album&gt;"
#define CDS_DIDL_ALBUM2_ESCAPED									"&lt;/upnp:album&gt;"
#define CDS_DIDL_ALBUM_ESCAPED_LEN								39

#define CDS_DIDL_GENRE1_ESCAPED									"\r\n&lt;upnp:genre&gt;"
#define CDS_DIDL_GENRE2_ESCAPED									"&lt;/upnp:genre&gt;"
#define CDS_DIDL_GENRE_ESCAPED_LEN								39

#define CDS_DIDL_CLASS1_ESCAPED									"\r\n&lt;upnp:class&gt;"
#define CDS_DIDL_CLASS2_ESCAPED									"&lt;/upnp:class&gt;"
#define CDS_DIDL_CLASS_ESCAPED_LEN								39

#define CDS_DIDL_CHANNELNAME1_ESCAPED							"\r\n&lt;upnp:channelName&gt;"
#define CDS_DIDL_CHANNELNAME2_ESCAPED							"&lt;/upnp:channelName&gt;"
#define CDS_DIDL_CHANNELNAME_ESCAPED_LEN						51

#define CDS_DIDL_CHANNELNR_ESCAPED								"\r\n&lt;upnp:channelNr&gt;%d&lt;/upnp:channelNr&gt;"
#define CDS_DIDL_CHANNELNR_ESCAPED_LEN							47

#define CDS_DIDL_DATE1_ESCAPED									"\r\n&lt;dc:date&gt;"
#define CDS_DIDL_DATE2_ESCAPED									"&lt;/dc:date&gt;"
#define CDS_DIDL_DATE_ESCAPED_LEN								33

#define CDS_DIDL_CONTAINERTYPE_ESCAPED							"\r\n&lt;dlna:containerType&gt;Tuner_1_0&lt;/dlna:containerType&gt;"
#define CDS_DIDL_CONTAINERTYPE_ESCAPED_LEN						64
	
#define CDS_DIDL_TAKEOUTGROUP_ESCAPED							"\r\n&lt;dlna:takeOut&gt;%s&lt;/dlna:takeOut&gt;"
#define CDS_DIDL_TAKEOUTGROUP_ESCAPED_LEN						43

#define	CDS_DIDL_DLNAMANAGED_ESCAPED							" dlnaManaged=&quot;"
#define	CDS_DIDL_DLNAMANAGED_ESCAPED_LEN						25

#define CDS_DIDL_DOUBLE_QUOTE_ESCAPED							"&quot;"
#define CDS_DIDL_ITEM_START1_ESCAPED							"\r\n&lt;item"
#define CDS_DIDL_ITEM_START2_ESCAPED							" id=&quot;"
#define CDS_DIDL_ITEM_START3_ESCAPED							" parentID=&quot;"
#define CDS_DIDL_ITEM_START4_ESCAPED							" restricted=&quot;"
#define CDS_DIDL_ITEM_START5_ESCAPED							" refID=&quot;"
#define CDS_DIDL_ITEM_START6_ESCAPED							"&gt;"
#define CDS_DIDL_ITEM_START_ESCAPED_LEN							95

#define CDS_DIDL_ITEM_END_ESCAPED								"\r\n&lt;/item&gt;"
#define CDS_DIDL_ITEM_END_ESCAPED_LEN							15

#define CDS_DIDL_CONTAINER_START1_ESCAPED						"\r\n&lt;container"
#define CDS_DIDL_CONTAINER_START2_ESCAPED						" id=&quot;"
#define CDS_DIDL_CONTAINER_START3_ESCAPED						" parentID=&quot;"
#define CDS_DIDL_CONTAINER_START4_ESCAPED						" restricted=&quot;"
#define CDS_DIDL_CONTAINER_START5_ESCAPED						" searchable=&quot;"
#define CDS_DIDL_CONTAINER_START6_ESCAPED						" childCount=&quot;%d"
#define CDS_DIDL_CONTAINER_START7_ESCAPED						"&gt;"
#define CDS_DIDL_CONTAINER_START_ESCAPED_LEN					129

#define CDS_DIDL_CONTAINER_END_ESCAPED							"\r\n&lt;/container&gt;"
#define CDS_DIDL_CONTAINER_END_ESCAPED_LEN						20

#define CDS_DIDL_CONTAINER_CREATECLASS1_ESCAPED					"\r\n&lt;upnp:createClass includeDerived=&quot;%s&quot;&gt;"
#define CDS_DIDL_CONTAINER_CREATECLASS2_ESCAPED					"&lt;/upnp:createClass&gt;"
#define CDS_DIDL_CONTAINER_CREATECLASS_ESCAPED_LEN				79

#define CDS_DIDL_CONTAINER_SEARCHCLASS1_ESCAPED					"\r\n&lt;upnp:searchClass includeDerived=&quot;%s&quot;&gt;"
#define CDS_DIDL_CONTAINER_SEARCHCLASS2_ESCAPED					"&lt;/upnp:searchClass&gt;"
#define CDS_DIDL_CONTAINER_SEARCHCLASS_ESCAPED_LEN				79

#define CDS_DIDL_RES1_ESCAPED									"\r\n&lt;res protocolInfo=&quot;"
#define CDS_DIDL_RES2_ESCAPED									"&quot;"
#define CDS_DIDL_RES_ESCAPED_LEN								35

#define CDS_DIDL_RES_ATTRIB_RESOLUTION_ESCAPED					" resolution=&quot;%dx%d&quot;"
#define CDS_DIDL_RES_ATTRIB_RESOLUTION_ESCAPED_LEN				24

#define CDS_DIDL_RES_ATTRIB_DURATION_ESCAPED					" duration=&quot;%s&quot;"
#define CDS_DIDL_RES_ATTRIB_DURATION_ESCAPED_LEN				22

#define CDS_DIDL_RES_ATTRIB_BITRATE_ESCAPED						" bitrate=&quot;%d&quot;"
#define CDS_DIDL_RES_ATTRIB_BITRATE_ESCAPED_LEN					21

#define CDS_DIDL_RES_ATTRIB_BITSPERSAMPLE_ESCAPED				" bitsPerSample=&quot;%d&quot;"
#define CDS_DIDL_RES_ATTRIB_BITSPERSAMPLE_ESCAPED_LEN			27

#define CDS_DIDL_RES_ATTRIB_COLORDEPTH_ESCAPED					" colorDepth=&quot;%d&quot;"
#define CDS_DIDL_RES_ATTRIB_COLORDEPTH_ESCAPED_LEN				24

#define CDS_DIDL_RES_ATTRIB_NRAUDIOCHANNELS_ESCAPED				" nrAudioChannels=&quot;%d&quot;"
#define CDS_DIDL_RES_ATTRIB_NRAUDIOCHANNELS_ESCAPED_LEN			29

#define CDS_DIDL_RES_ATTRIB_PROTECTION_ESCAPED					" protection=&quot;%s&quot;"
#define CDS_DIDL_RES_ATTRIB_PROTECTION_ESCAPED_LEN				24

#define CDS_DIDL_RES_ATTRIB_SAMPLEFREQUENCY_ESCAPED				" sampleFrequency=&quot;%d&quot;"
#define CDS_DIDL_RES_ATTRIB_SAMPLEFREQUENCY_ESCAPED_LEN			29

#define CDS_DIDL_RES_ATTRIB_SIZE_ESCAPED						" size=&quot;%ld&quot;"
#define CDS_DIDL_RES_ATTRIB_SIZE_ESCAPED_LEN					18

#define CDS_DIDL_RES_ATTRIB_IMPORTURI1_ESCAPED					" importUri=&quot;"
#define CDS_DIDL_RES_ATTRIB_IMPORTURI2_ESCAPED					"&quot;"
#define CDS_DIDL_RES_ATTRIB_IMPORTURI_ESCAPED_LEN				23

#define CDS_DIDL_RES_ATTRIB_IFOFILEURI1_ESCAPED					" dlna:ifoFileURI=&quot;"
#define CDS_DIDL_RES_ATTRIB_IFOFILEURI2_ESCAPED					"&quot;"
#define CDS_DIDL_RES_ATTRIB_IFOFILEURI_ESCAPED_LEN				29

#define CDS_DIDL_RES_ATTRIB_IMPORTIFOFILEURI1_ESCAPED			" importIfoFileUri=&quot;"
#define CDS_DIDL_RES_ATTRIB_IMPORTIFOFILEURI2_ESCAPED			"&quot;"
#define CDS_DIDL_RES_ATTRIB_IMPORTIFOFILEURI_ESCAPED_LEN		30

#define CDS_DIDL_RES_ATTRIB_RESUMEUPLOAD_ESCAPED				" dlna:resumeUpload=&quot;%d&quot;"
#define CDS_DIDL_RES_ATTRIB_RESUMEUPLOAD_ESCAPED_LEN			31

#define CDS_DIDL_RES_ATTRIB_UPLOADEDSIZE_ESCAPED				" dlna:uploadedSize=&quot;%d&quot;"
#define CDS_DIDL_RES_ATTRIB_UPLOADEDSIZE_ESCAPED_LEN			31

#define CDS_DIDL_RES_ATTRIB_TRACKTOTAL_ESCAPED					" dlna:trackTotal=&quot;%d&quot;"
#define CDS_DIDL_RES_ATTRIB_TRACKTOTAL_ESCAPED_LEN				29

#define CDS_DIDL_RES_VALUE1_ESCAPED								"&gt;"
#define CDS_DIDL_RES_VALUE2_ESCAPED								"&lt;/res&gt;"
#define CDS_DIDL_RES_VALUE_ESCAPED_LEN							16


/* CDS normative tag names and attributes for DIDL to CDS Object */
#define CDS_XML_NAMESPACE_DIDL					"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/"
#define CDS_XML_NAMESPACE_DIDL_LEN				44
#define CDS_XML_NAMESPACE_DC					"http://purl.org/dc/elements/1.1/"
#define CDS_XML_NAMESPACE_DC_LEN				32
#define CDS_XML_NAMESPACE_UPNP					"urn:schemas-upnp-org:metadata-1-0/upnp/"
#define CDS_XML_NAMESPACE_UPNP_LEN				39
#define CDS_XML_NAMESPACE_DLNA					"http://mediabolic.com/schema/xmb_1.0/"
#define CDS_XML_NAMESPACE_DLNA_LEN				37

#define CDS_ATTRIB_ID							"id"
#define CDS_ATTRIB_PARENTID						"parentID"
#define CDS_ATTRIB_REFID						"refID"
#define CDS_ATTRIB_RESTRICTED					"restricted"
#define CDS_ATTRIB_SEARCHABLE					"searchable"
#define CDS_ATTRIB_PROTOCOLINFO					"protocolInfo"
#define CDS_ATTRIB_RESOLUTION					"resolution"
#define CDS_ATTRIB_DURATION						"duration"
#define CDS_ATTRIB_BITRATE						"bitrate"
#define CDS_ATTRIB_BITSPERSAMPLE				"bitsPerSample"
#define CDS_ATTRIB_COLORDEPTH					"colorDepth"
#define CDS_ATTRIB_NRAUDIOCHANNELS				"nrAudioChannels"
#define CDS_ATTRIB_PROTECTION					"protection"
#define CDS_ATTRIB_SAMPLEFREQUENCY				"sampleFrequency"
#define CDS_ATTRIB_SIZE							"size"
#define CDS_ATTRIB_DLNAMANAGED					"dlnaManaged"
#define CDS_ATTRIB_IMPORTURI					"importUri"
#define CDS_ATTRIB_IFOFILEURI					"ifoFileUri"
#define CDS_ATTRIB_IMPORTIFOFILEURI				"importIfoFileUri"
#define CDS_ATTRIB_RESUMEUPLOAD					"resumeUpload"
#define CDS_ATTRIB_UPLOADEDSIZE					"uploadedSize"
#define CDS_ATTRIB_TRACKTOTAL					"trackTotal"
#define CDS_ATTRIB_INCLUDEDERIVED				"includeDerived"

#define CDS_TAG_DIDL							"DIDL-Lite"
#define CDS_TAG_CONTAINER						"container"
#define CDS_TAG_ITEM							"item"
#define CDS_TAG_RESOURCE						"res"

#define CDS_TAG_CREATOR							"creator"
#define CDS_TAG_TITLE							"title"
#define CDS_TAG_DATE							"date"
#define CDS_TAG_MEDIACLASS						"class"
#define CDS_TAG_CREATECLASS						"createClass"
#define CDS_TAG_SEARCHCLASS						"searchClass"
#define CDS_TAG_GENRE							"genre"
#define CDS_TAG_ALBUM							"album"
#define CDS_TAG_CHANNELNAME						"channelName"
#define CDS_TAG_CHANNELNR						"channelNr"
#define CDS_TAG_CONTAINERTYPE					"containerType"
#define CDS_TAG_TAKEOUTGROUP					"takeOut"

#define CDS_TRUE_STRINGS_LEN					3
#define CDS_FALSE_STRINGS_LEN					3

#endif
