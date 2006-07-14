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
 * $Workfile: CdsDidlSerializer.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "CdsDidlSerializer.h"
#include "CdsStrings.h"
#include "CdsMediaClass.h"
#include "ILibParsers.h"

#ifdef MSCP
#include "MediaServerControlPoint.h"
#endif

#ifdef WIN32
#define strncasecmp(x,y,z) _strnicmp(x,y,z)
#endif

#ifdef _WIN32_WCE
#define strncasecmp(x,y,z) _strnicmp(x,y,z)
#endif


#ifndef _WIN32_WCE
	#include <assert.h>
	#define ASSERT(x) assert(x)
#endif

#define MAX_TIME_STRING_SIZE 17

/*
 *	Maximum length of a string in 
 *	MSCP_CLASS_OBJECT_TYPE, MSCP_CLASS_MAJOR_TYPE, and MSCP_CLASS_MINOR_TYPE.
 *	Size includes null terminator.
 */
#define CDS_MAX_CLASS_FRAGMENT_LEN	17
#define CDS_MAX_CLASS_FRAGMENT_SIZE	18

#define MIN(X, Y)  ((X) < (Y) ? (X) : (Y))

typedef int (*CdsToDidl_Fn_XmlEscapeLength) (const char* string);
typedef int (*CdsToDidl_Fn_XmlEscape) (char *dest, const char* src);

const char* CDS_TRUE_STRINGS[] = {"1", "true", "yes"};
const char* CDS_FALSE_STRINGS[] = {"0", "false", "no"};


/*
 *	Helper function.
 *	Returns the length for the specified string in its doubly-escaped form.
 */
int _CdsToDidl_Helper_DoubleEscapeLength(const char* data)
{
	int i = 0, j = 0;
	while (data[i] != 0)
	{
		switch (data[i])
		{
			case '"':
			j += 10;
			break;
			case '\'':
			j += 10;
			break;
			case '<':
			j += 8;
			break;
			case '>':
			j += 8;
			break;
			case '&':
			j += 9;
			break;
			default:
			j++;
		}
		i++;
	}
	return j;
}

int _CdsToDidl_Helper_DoubleEscape(char* outdata, const char* indata)
{
	int i=0;
	int inlen;
	char* out;
	
	out = outdata;
	inlen = (int)strlen(indata);
	
	for (i=0; i < inlen; i++)
	{
		if (indata[i] == '"')
		{
			memcpy(out, "&amp;quot;", 10);
			out = out + 10;
		}
		else
		if (indata[i] == '\'')
		{
			memcpy(out, "&amp;apos;", 10);
			out = out + 10;
		}
		else
		if (indata[i] == '<')
		{
			memcpy(out, "&amp;lt;", 8);
			out = out + 8;
		}
		else
		if (indata[i] == '>')
		{
			memcpy(out, "&amp;gt;", 8);
			out = out + 8;
		}
		else
		if (indata[i] == '&')
		{
			memcpy(out, "&amp;amp;", 9);
			out = out + 9;
		}
		else
		{
			out[0] = indata[i];
			out++;
		}
	}
	
	out[0] = 0;
	
	return (int)(out - outdata);
}

/*
 *	Prints the number of seconds in hh:mm:ss format.
 *	Negative values do not write anything.
 */
void _CdsToDidl_Helper_WriteDurationString(char* str, int intTime)
{
	if (intTime >= 0)
	{
		sprintf(str,"%d:%02d:%02d.000",(intTime/3600),((intTime/60)%60),(intTime%60));
	}
}

int _CdsToDidl_Helper_ParseDurationString(char* duration)
{
	char *hour=NULL,*minute=NULL,*second=NULL;
	int numOfHours;
	int numOfMinutes;
	int numOfSeconds;
	struct parser_result *pr;
	int totalSeconds = 0;

	pr = ILibParseString(duration,0,(int)strlen(duration),":",1);
	if(pr->NumResults==3)
	{
		hour = pr->FirstResult->data;
		hour[pr->FirstResult->datalength]=0;
		minute = pr->FirstResult->NextResult->data;
		minute[pr->FirstResult->NextResult->datalength]=0;
		second = pr->FirstResult->NextResult->NextResult->data;
		second[2]=0; // we are going to ignore the faction at the end.

		ILibDestructParserResults(pr);
	}
	if(hour!=NULL && minute!=NULL && second!=NULL)
	{
		numOfHours = atoi(hour);
		numOfMinutes = atoi(minute);
		numOfSeconds = atoi(second);
		totalSeconds = numOfHours * 3600 + numOfMinutes * 60 + numOfSeconds;
	}

	return totalSeconds;
}

int _CdsToDidl_Helper_GetSizeForSingleString(CdsToDidl_Fn_XmlEscapeLength fn_escape_length, unsigned int filter, unsigned int filter_mask, int tag_name_len, char *string, /*INOUT*/ unsigned int *print_these)
{
	int retval = 0;

	if ((filter & filter_mask) && (string != NULL))
	{
		retval += (tag_name_len + fn_escape_length(string));

		*print_these |= filter_mask;
	}

	return retval;
}

int _CdsToDidl_Helper_GetSizeForMultipleStrings(CdsToDidl_Fn_XmlEscapeLength fn_escape_length, unsigned int filter, unsigned int filter_mask, int tag_name_len, int num_strings, char **strings, /*INOUT*/ unsigned int *print_these)
{
	int i;
	int retval = 0;

	if ((filter & filter_mask) && (num_strings > 0) && (strings != NULL))
	{
		for (i=0; i < num_strings; i++)
		{
			if (strings[i] != NULL)
			{
				retval += (tag_name_len + fn_escape_length(strings[i]));
			}
		}

		*print_these |= filter_mask;
	}

	return retval;
}

int _CdsToDidl_Helper_GetSizeForDate(unsigned int filter, unsigned int filter_mask, int tag_name_len, unsigned int date_val, /*INOUT*/ unsigned int *print_these)
{
	int retval = 0;
	if ((filter & filter_mask) && (date_val > 0))
	{
		retval += (tag_name_len + CDS_DATE_VALUE_LEN);
		*print_these |= filter_mask;
	}
	return retval;
}

int _CdsToDidl_Helper_GetSizeForNonNegativeInt(unsigned int filter, unsigned int filter_mask, int tag_name_len, int non_negative, /*INOUT*/ unsigned int *print_these)
{
	int retval = 0;
	if ((filter & filter_mask) && (non_negative >= 0))
	{
		retval += (tag_name_len + CDS_NUMBER_VALUE_LEN);
		*print_these |= filter_mask;
	}
	return retval;
}

int _CdsToDidl_Helper_GetSizeForItem(CdsToDidl_Fn_XmlEscapeLength fn_escape_length, struct CdsObject *cds_obj, unsigned int filter, unsigned int *print_these)
{
	int retval = 0;
	
	if (cds_obj->TypeObject.Item.RefID != NULL)
	{
		retval = fn_escape_length(cds_obj->TypeObject.Item.RefID);
	}
	
	return retval;
}

int _CdsToDidl_Helper_GetSizeForAudioItem(CdsToDidl_Fn_XmlEscapeLength fn_escape_length, struct CdsObject *cds_obj, unsigned int filter, unsigned int *print_these)
{
	int retval = 0;

	retval += _CdsToDidl_Helper_GetSizeForMultipleStrings(
		fn_escape_length, 
		filter,
		CdsFilter_Album,
		CDS_DIDL_ALBUM_ESCAPED_LEN,
		cds_obj->TypeMajor.AudioItem.NumAlbums,
		cds_obj->TypeMajor.AudioItem.Albums,
		print_these);

	retval += _CdsToDidl_Helper_GetSizeForMultipleStrings(
		fn_escape_length, 
		filter,
		CdsFilter_Genre,
		CDS_DIDL_GENRE_ESCAPED_LEN,
		cds_obj->TypeMajor.AudioItem.NumGenres,
		cds_obj->TypeMajor.AudioItem.Genres,
		print_these);

	retval += _CdsToDidl_Helper_GetSizeForDate(
		filter, 
		CdsFilter_Date,
		CDS_DIDL_DATE_ESCAPED_LEN,
		cds_obj->TypeMajor.AudioItem.Date,
		print_these);
	return retval;
}

int _CdsToDidl_Helper_GetSizeForImageItem(CdsToDidl_Fn_XmlEscapeLength fn_escape_length, struct CdsObject *cds_obj, unsigned int filter, unsigned int *print_these)
{
	int retval = 0;

	retval += _CdsToDidl_Helper_GetSizeForDate(
		filter, 
		CdsFilter_Date,
		CDS_DIDL_DATE_ESCAPED_LEN,
		cds_obj->TypeMajor.ImageItem.Date,
		print_these);
	
	return retval;
}

int _CdsToDidl_Helper_GetSizeForVideoItem(CdsToDidl_Fn_XmlEscapeLength fn_escape_length, struct CdsObject *cds_obj, unsigned int filter, unsigned int *print_these)
{
	int retval = 0;

	retval += _CdsToDidl_Helper_GetSizeForMultipleStrings(
		fn_escape_length, 
		filter,
		CdsFilter_Genre,
		CDS_DIDL_GENRE_ESCAPED_LEN,
		cds_obj->TypeMajor.VideoItem.NumGenres,
		cds_obj->TypeMajor.VideoItem.Genres,
		print_these);

	retval += _CdsToDidl_Helper_GetSizeForDate(
		filter, 
		CdsFilter_Date,
		CDS_DIDL_DATE_ESCAPED_LEN,
		cds_obj->TypeMajor.VideoItem.Date,
		print_these);
	
	return retval;
}

int _CdsToDidl_Helper_GetSizeForMusicAlbum(CdsToDidl_Fn_XmlEscapeLength fn_escape_length, struct CdsObject *cds_obj, unsigned int filter, unsigned int *print_these)
{
	int retval = 0;

	retval += _CdsToDidl_Helper_GetSizeForMultipleStrings(
		fn_escape_length, 
		filter,
		CdsFilter_Genre,
		CDS_DIDL_GENRE_ESCAPED_LEN,
		cds_obj->TypeMinor1.MusicAlbum.NumGenres,
		cds_obj->TypeMinor1.MusicAlbum.Genres,
		print_these);
	
	return retval;
}

int _CdsToDidl_Helper_GetSizeForPhoto(CdsToDidl_Fn_XmlEscapeLength fn_escape_length, struct CdsObject *cds_obj, unsigned int filter, unsigned int *print_these)
{
	int retval = 0;

	retval += _CdsToDidl_Helper_GetSizeForMultipleStrings(
		fn_escape_length, 
		filter,
		CdsFilter_Album,
		CDS_DIDL_ALBUM_ESCAPED_LEN,
		cds_obj->TypeMinor1.Photo.NumAlbums,
		cds_obj->TypeMinor1.Photo.Albums,
		print_these);
	
	return retval;
}

int _CdsToDidl_Helper_GetSizeForAudioBroadcast(CdsToDidl_Fn_XmlEscapeLength fn_escape_length, struct CdsObject *cds_obj, unsigned int filter, unsigned int *print_these)
{
	int retval = 0;

	retval += _CdsToDidl_Helper_GetSizeForSingleString(
		fn_escape_length, 
		filter,
		CdsFilter_ChannelName,
		CDS_DIDL_CHANNELNAME_ESCAPED_LEN,
		cds_obj->TypeMinor1.AudioBroadcast.ChannelName,
		print_these);

	retval += _CdsToDidl_Helper_GetSizeForNonNegativeInt(
		filter,
		CdsFilter_ChannelNr,
		CDS_DIDL_CHANNELNR_ESCAPED_LEN,
		cds_obj->TypeMinor1.AudioBroadcast.ChannelNr,
		print_these);
	
	return retval;
}

int _CdsToDidl_Helper_GetSizeForVideoBroadcast(CdsToDidl_Fn_XmlEscapeLength fn_escape_length, struct CdsObject *cds_obj, unsigned int filter, unsigned int *print_these)
{
	int retval = 0;

	retval += _CdsToDidl_Helper_GetSizeForSingleString(
		fn_escape_length, 
		filter,
		CdsFilter_ChannelName,
		CDS_DIDL_CHANNELNAME_ESCAPED_LEN,
		cds_obj->TypeMinor1.VideoBroadcast.ChannelName,
		print_these);

	retval += _CdsToDidl_Helper_GetSizeForNonNegativeInt(
		filter,
		CdsFilter_ChannelNr,
		CDS_DIDL_CHANNELNR_ESCAPED_LEN,
		cds_obj->TypeMinor1.VideoBroadcast.ChannelNr,
		print_these);
	
	return retval;
}

int _CdsToDidl_Helper_GetSizeForTakeOutGroups(CdsToDidl_Fn_XmlEscapeLength fn_escape_length, struct CdsObject *cds_obj, unsigned int filter, unsigned int *print_these)
{
	int retval = 0;

	retval += _CdsToDidl_Helper_GetSizeForMultipleStrings(
		fn_escape_length, 
		filter,
		CdsFilter_TakeOut,
		CDS_DIDL_TAKEOUTGROUP_ESCAPED_LEN,
		cds_obj->NumTakeOutGroups,
		cds_obj->TakeOutGroups,
		print_these);
	
	return retval;
}

/* see header file */
unsigned int CDS_ConvertCsvStringToBitString(const char *filter)
{
	unsigned int retVal = 0;
	int i=0;
	int nextComma = 0;
	int filterLen = 0;

	/*
	*	SOLUTION_REFERENCE#3.6.3.2a
	*/

	if (filter != NULL)
	{
		filterLen = (int) strlen(filter);
		if (filterLen > 0)
		{
			/*
 			 *	The filter string is comma-delimited.
			 *	Do a linear parse and set bits in the retVal
			 *	for supported fields.
			 *
			 *	All supported, filterable metadata fields begin with
			 *	dc:, upnp:, res, container@, or @.
			 */

			i = nextComma = 0;
			while (filter[i] != '\0')
			{
				if (nextComma == 0)
				{
					switch (tolower(filter[i]))
					{
					case 'a':
						nextComma = 1;
						/* only supported filterable field is ALLIP, which is a DHWG extension */
						if (strncasecmp(filter+i, CDS_STRING_ALLIP, CDS_STRING_ALLIP_LEN) == 0)
						{
							retVal |= CdsFilter_DhwgAllIp;
						}
						break;

					case '@':
						nextComma = 1;
						/* Only supported filterable field are container@childCount and container@searchable */
						if (strncasecmp(filter+i, CDS_STRING_CHILDCOUNT, CDS_STRING_CHILDCOUNT_LEN) == 0)
						{
							retVal |= CdsFilter_ChildCount;
						}
						else if (strncasecmp(filter+i, CDS_STRING_SEARCHABLE, CDS_STRING_SEARCHABLE_LEN) == 0)
						{
							retVal |= CdsFilter_Searchable;
						}
						else if (strncasecmp(filter+i, CDS_STRING_DLNAMANAGED, CDS_STRING_DLNAMANAGED_LEN) == 0)
						{
							retVal |= CdsFilter_DlnaManaged;
						}
						break;

					case 'c':
						nextComma = 1;
						/* Only supported filterable field are container@childCount and container@searchable */
						if (strncasecmp(filter+i, CDS_STRING_CONTAINER_CHILDCOUNT, CDS_STRING_CONTAINER_CHILDCOUNT_LEN) == 0)
						{
							retVal |= CdsFilter_ChildCount;
						}
						else if (strncasecmp(filter+i, CDS_STRING_CONTAINER_SEARCHABLE, CDS_STRING_CONTAINER_SEARCHABLE_LEN) == 0)
						{
							retVal |= CdsFilter_Searchable;
						}
						break;

					case 'd':
						nextComma = 1;

						/* Only supported filterable field is dc:creator and dc:date. (dc:title is always required.) */
						if (strncasecmp(filter+i, CDS_STRING_CREATOR, CDS_STRING_CREATOR_LEN) == 0)
						{
							retVal |= CdsFilter_Creator;
						}
						if (strncasecmp(filter+i, CDS_STRING_DATE, CDS_STRING_DATE_LEN) == 0)
						{
							retVal |= CdsFilter_Date;
						}
						break;

					case 'u':
						/* Only supported filterable field is upnp:album and upnp:genre */
						if (strncasecmp(filter+i, CDS_STRING_ALBUM, CDS_STRING_ALBUM_LEN) == 0)
						{
							retVal |= CdsFilter_Album;
						}
						else if (strncasecmp(filter+i, CDS_STRING_GENRE, CDS_STRING_GENRE_LEN) == 0)
						{
							retVal |= CdsFilter_Genre;
						}
						nextComma = 1;
						break;

					case 'r':
						nextComma = 1;
						/* only supported fields are: res, resolution, duration, bitrate, colordepth, size */
						
						if (strncasecmp(filter+i, CDS_STRING_RES_BITRATE, CDS_STRING_RES_BITRATE_LEN) == 0)
						{
							retVal |= CdsFilter_Res;
							retVal |= CdsFilter_ResBitrate;
						}
						else if (strncasecmp(filter+i, CDS_STRING_RES_BITSPERSAMPLE, CDS_STRING_RES_BITSPERSAMPLE_LEN) == 0)
						{
							retVal |= CdsFilter_Res;
							retVal |= CdsFilter_ResBitsPerSample;
						}
						else if (strncasecmp(filter+i, CDS_STRING_RES_COLORDEPTH, CDS_STRING_RES_COLORDEPTH_LEN) == 0)
						{
							retVal |= CdsFilter_Res;
							retVal |= CdsFilter_ResColorDepth;
						}
						else if (strncasecmp(filter+i, CDS_STRING_RES_DURATION, CDS_STRING_RES_DURATION_LEN) == 0)
						{
							retVal |= CdsFilter_Res;
							retVal |= CdsFilter_ResDuration;
						}
						else if (strncasecmp(filter+i, CDS_STRING_RES_NRAUDIOCHANNELS, CDS_STRING_RES_NRAUDIOCHANNELS_LEN) == 0)
						{
							retVal |= CdsFilter_Res;
							retVal |= CdsFilter_ResNrAudioChannels;
						}
						else if (strncasecmp(filter+i, CDS_STRING_RES_PROTECTION, CDS_STRING_RES_PROTECTION_LEN) == 0)
						{
							retVal |= CdsFilter_Res;
							retVal |= CdsFilter_ResProtection;
						}
						else if (strncasecmp(filter+i, CDS_STRING_RES_RESOLUTION, CDS_STRING_RES_RESOLUTION_LEN) == 0)
						{
							retVal |= CdsFilter_Res;
							retVal |= CdsFilter_ResResolution;
						}
						else if (strncasecmp(filter+i, CDS_STRING_RES_SAMPLEFREQUENCY, CDS_STRING_RES_SAMPLEFREQUENCY_LEN) == 0)
						{
							retVal |= CdsFilter_Res;
							retVal |= CdsFilter_ResSampleFrequency;
						}
						else if (strncasecmp(filter+i, CDS_STRING_RES_SIZE, CDS_STRING_RES_SIZE_LEN) == 0)
						{
							retVal |= CdsFilter_Res;
							retVal |= CdsFilter_ResSize;
						}
						else if (strncasecmp(filter+i, CDS_STRING_RES_IMPORTURI, CDS_STRING_RES_IMPORTURI_LEN) == 0)
						{
							retVal |= CdsFilter_Res;
							retVal |= CdsFilter_ResImportUri;
						}
						else if (strncasecmp(filter+i, CDS_STRING_RES_IFOFILEURI, CDS_STRING_RES_IFOFILEURI_LEN) == 0)
						{
							retVal |= CdsFilter_Res;
							retVal |= CdsFilter_ResIfoFileUri;
						}
						else if (strncasecmp(filter+i, CDS_STRING_RES_IMPORTIFOFILEURI, CDS_STRING_RES_IMPORTIFOFILEURI_LEN) == 0)
						{
							retVal |= CdsFilter_Res;
							retVal |= CdsFilter_ResImportIfoFileUri;
						}
						else if (strncasecmp(filter+i, CDS_STRING_RES_RESUMEUPLOAD, CDS_STRING_RES_RESUMEUPLOAD_LEN) == 0)
						{
							retVal |= CdsFilter_Res;
							retVal |= CdsFilter_ResResumeUpload;
						}
						else if (strncasecmp(filter+i, CDS_STRING_RES_UPLOADEDSIZE, CDS_STRING_RES_UPLOADEDSIZE_LEN) == 0)
						{
							retVal |= CdsFilter_Res;
							retVal |= CdsFilter_ResUploadedSize;
						}
						else if (strncasecmp(filter+i, CDS_STRING_RES_TRACKTOTAL, CDS_STRING_RES_TRACKTOTAL_LEN) == 0)
						{
							retVal |= CdsFilter_Res;
							retVal |= CdsFilter_ResTrackTotal;
						}


						else if (strncasecmp(filter+i, CDS_STRING_RES, CDS_STRING_RES_LEN) == 0)
						{
							/* "res" must be tested after all res attributes */
							retVal |= CdsFilter_Res;
						}
						break;

					case '*':
						/* asterik infers all except ALLIP */
						retVal |= (~CdsFilter_DhwgAllIp);
						break;

					default:
						/*
						 *	If the character is an alphabetic char, then go to the next comma
						 *	because we know that this character is not for something we support.
						 */
						if (isalpha(filter[i]))
						{
							nextComma = 1;
						}
					}
				}
				else
				{
					/* We're trying to find the next comma... so if we find it, set the nextComma flag to zero. */
					if (filter[i] == ',')
					{
						nextComma = 0;
					}
				}

				/* move to the next char */
				i++;
			}
		}
	}

	return retVal;
}

/* see header file */
int CDS_GetCsvLengthFromBitString(unsigned int bitstring)
{
	int length = 0;
	int numFields = 0;

	if (bitstring & CdsSearchSort_Title)			{ length += CDS_STRING_TITLE_LEN;				numFields++; }
	if (bitstring & CdsFilter_Creator)				{ length += CDS_STRING_CREATOR_LEN;				numFields++; }
	
	if (bitstring & CdsFilter_ChildCount)			{ length += CDS_STRING_CHILDCOUNT_LEN;			numFields++; }
	if (bitstring & CdsFilter_Searchable)			{ length += CDS_STRING_SEARCHABLE_LEN;			numFields++; }
	if (bitstring & CdsFilter_DlnaManaged)			{ length += CDS_STRING_DLNAMANAGED_LEN;			numFields++; }

	if (bitstring & CdsFilter_Album)				{ length += CDS_STRING_ALBUM_LEN;				numFields++; }
	if (bitstring & CdsFilter_Genre)				{ length += CDS_STRING_GENRE_LEN;				numFields++; }

	if (bitstring & CdsFilter_Res)					{ length += CDS_STRING_RES_LEN;					numFields++; }
	if (bitstring & CdsFilter_ResResolution)		{ length += CDS_STRING_RES_RESOLUTION_LEN;		numFields++; }
	if (bitstring & CdsFilter_ResDuration)			{ length += CDS_STRING_RES_DURATION_LEN;		numFields++; }
	if (bitstring & CdsFilter_ResBitrate)			{ length += CDS_STRING_RES_BITRATE_LEN;			numFields++; }
	if (bitstring & CdsFilter_ResColorDepth)		{ length += CDS_STRING_RES_COLORDEPTH_LEN;		numFields++; }
	if (bitstring & CdsFilter_ResSize)				{ length += CDS_STRING_RES_SIZE_LEN;			numFields++; }
	if (bitstring & CdsFilter_ResBitsPerSample)		{ length += CDS_STRING_RES_BITSPERSAMPLE_LEN;	numFields++; }
	if (bitstring & CdsFilter_ResSampleFrequency)	{ length += CDS_STRING_RES_SAMPLEFREQUENCY_LEN; numFields++; }
	if (bitstring & CdsFilter_ResNrAudioChannels)	{ length += CDS_STRING_RES_NRAUDIOCHANNELS_LEN; numFields++; }
	if (bitstring & CdsFilter_ResProtection)		{ length += CDS_STRING_RES_PROTECTION_LEN;		numFields++; }

	if (numFields > 1)
	{
		/*
		 *	if there is more than 1 field, we have to include 
		 *	bytes for the comma chacters between the fields
		 */
		length += numFields;
		length--;
	}

	return length;
}

/* see header file */
#define WRITE_CSV_VALUE(sh,st,n) \
	if (n>0) { sh += sprintf(sh,",%s", st); }\
	else { sh += sprintf(sh,"%s", st); }\
	n++;
char* CDS_ConvertBitStringToCsvString(char *csvString, unsigned int bitstring)
{
	int numElements = 0;
	char *csv = csvString;

	if (bitstring & CdsSearchSort_Title) { WRITE_CSV_VALUE(csv, CDS_STRING_TITLE, numElements); }
	if (bitstring & CdsFilter_Creator) { WRITE_CSV_VALUE(csv, CDS_STRING_CREATOR, numElements); }
	
	if (bitstring & CdsFilter_ChildCount) { WRITE_CSV_VALUE(csv, CDS_STRING_CHILDCOUNT, numElements); }
	if (bitstring & CdsFilter_Searchable) { WRITE_CSV_VALUE(csv, CDS_STRING_SEARCHABLE, numElements); }
	if (bitstring & CdsFilter_DlnaManaged) { WRITE_CSV_VALUE(csv, CDS_STRING_DLNAMANAGED, numElements); }

	if (bitstring & CdsFilter_Album) { WRITE_CSV_VALUE(csv, CDS_STRING_ALBUM, numElements); }
	if (bitstring & CdsFilter_Genre) { WRITE_CSV_VALUE(csv, CDS_STRING_GENRE, numElements); }

	if (bitstring & CdsFilter_Res) { WRITE_CSV_VALUE(csv, CDS_STRING_RES, numElements); }
	if (bitstring & CdsFilter_ResBitrate) { WRITE_CSV_VALUE(csv, CDS_STRING_RES_BITRATE, numElements); }
	if (bitstring & CdsFilter_ResBitsPerSample) { WRITE_CSV_VALUE(csv, CDS_STRING_RES_BITSPERSAMPLE, numElements); }
	if (bitstring & CdsFilter_ResColorDepth) { WRITE_CSV_VALUE(csv, CDS_STRING_RES_COLORDEPTH, numElements); }
	if (bitstring & CdsFilter_ResDuration) { WRITE_CSV_VALUE(csv, CDS_STRING_RES_DURATION, numElements); }
	if (bitstring & CdsFilter_ResProtection) { WRITE_CSV_VALUE(csv, CDS_STRING_RES_PROTECTION, numElements); }
	if (bitstring & CdsFilter_ResResolution) { WRITE_CSV_VALUE(csv, CDS_STRING_RES_RESOLUTION, numElements); }
	if (bitstring & CdsFilter_ResSampleFrequency) { WRITE_CSV_VALUE(csv, CDS_STRING_RES_SAMPLEFREQUENCY, numElements); }
	if (bitstring & CdsFilter_ResNrAudioChannels) { WRITE_CSV_VALUE(csv, CDS_STRING_RES_NRAUDIOCHANNELS, numElements); }
	if (bitstring & CdsFilter_ResSize) { WRITE_CSV_VALUE(csv, CDS_STRING_RES_SIZE, numElements); }
	
	if (bitstring & CdsFilter_ResImportUri) { WRITE_CSV_VALUE(csv, CDS_STRING_RES_IMPORTURI, numElements); }
	if (bitstring & CdsFilter_ResIfoFileUri) { WRITE_CSV_VALUE(csv, CDS_STRING_RES_IFOFILEURI, numElements); }
	if (bitstring & CdsFilter_ResImportIfoFileUri) { WRITE_CSV_VALUE(csv, CDS_STRING_RES_IMPORTIFOFILEURI, numElements); }
	if (bitstring & CdsFilter_ResResumeUpload) { WRITE_CSV_VALUE(csv, CDS_STRING_RES_RESUMEUPLOAD, numElements); }
	if (bitstring & CdsFilter_ResUploadedSize) { WRITE_CSV_VALUE(csv, CDS_STRING_RES_UPLOADEDSIZE, numElements); }
	if (bitstring & CdsFilter_ResTrackTotal) { WRITE_CSV_VALUE(csv, CDS_STRING_RES_TRACKTOTAL, numElements); }

	csv[0] = '\0';
	return csvString;
}

/* see header file */
char* CDS_SerializeObjectToDidl (struct CdsObject *mediaObj, int cds_obj_metadata_escaped, unsigned int filter, int includeHeaderFooter, int *outErrorOrDidlLen)
{
	char *retVal = NULL;	
	int size = 1;			/* allocation size of retVal: minimum of a null character needed; if value is ever negative, then there's an error. */
	int len = 0;			/* actual length of the string in retVal */
	int imc_Object = -1, imc_Major = -1, imc_Minor1 = -1, imc_Minor2 = -1; /* index into the various CDS_CLASS_xxx string arrays */
	struct CdsResource *res;
	struct CdsCreateClass *createClass;
	struct CdsSearchClass *searchClass;
	unsigned int printThese = 0;		/* similar to filter, except indicates which fields will actually get printed */
	char *cp = NULL;
	char* timeStr = NULL;
	char durationStr[30];
	int i=0; //loop through takeout groups

	CdsToDidl_Fn_XmlEscapeLength fnEscapeLength;
	CdsToDidl_Fn_XmlEscape		 fnEscape;

	/*
	 *	Given the info about the metadata,
	 *	obtain function pointers to the appropriate
	 *	functions for calculating lengths and escaping strings.
	 */
	if (cds_obj_metadata_escaped)
	{
		fnEscapeLength = ILibXmlEscapeLength;
		fnEscape = ILibXmlEscape;
	}
	else
	{
		fnEscapeLength = _CdsToDidl_Helper_DoubleEscapeLength;
		fnEscape = _CdsToDidl_Helper_DoubleEscape;
	}


	/*
	 *	Include length needed for metadata content.
	 *	We need to doubly-escape because the data is a value of an XML element or attribute,
	 *	but the DIDL-Lite XML itself needs to be escaped, so this means all of the
	 *	values of XML attributes and elements need escaping too.
	 *
	 *	If at any point we determine there's an error with the media object, we
	 *	set "size" to an appropriate negative error value.
	 */
	if ((mediaObj->ID != NULL))
	{
		size += fnEscapeLength(mediaObj->ID);
	}

	if ((mediaObj->ParentID != NULL) && (strlen(mediaObj->ParentID) > 0))
	{
		size += fnEscapeLength(mediaObj->ParentID);	
	}

	if ((mediaObj->Title != NULL) && (strlen(mediaObj->Title) > 0))
	{
		size += (CDS_DIDL_TITLE_ESCAPED_LEN + fnEscapeLength(mediaObj->Title));
	}

	/* ObjectID, ParentID, and Title are valid... */

	/* do not add memory for creator unless requested by the metadata filter */
	if ((mediaObj->Creator != NULL) && (filter & CdsFilter_Creator))
	{
		size += (CDS_DIDL_CREATOR_ESCAPED_LEN + fnEscapeLength(mediaObj->Creator));
		printThese |= CdsFilter_Creator;
	}

	switch (mediaObj->MediaClass & CDS_CLASS_MASK_OBJECT_TYPE)
	{
		case CDS_CLASS_MASK_ITEM:
			size += _CdsToDidl_Helper_GetSizeForItem(fnEscapeLength, mediaObj, filter, &printThese);
			break;

			/*	TODO: If containers ever have string data specific to them
				then you need to add appropriate deallocator code here.

		case CDS_CLASS_MASK_CONTAINER:
			break;
			*/
	}

	switch (mediaObj->MediaClass & CDS_CLASS_MASK_MAJOR)
	{
		case CDS_CLASS_MASK_MAJOR_AUDIOITEM:
			size += _CdsToDidl_Helper_GetSizeForAudioItem(fnEscapeLength, mediaObj, filter, &printThese);
			break;
		case CDS_CLASS_MASK_MAJOR_IMAGEITEM:
			size += _CdsToDidl_Helper_GetSizeForImageItem(fnEscapeLength, mediaObj, filter, &printThese);
			break;
		case CDS_CLASS_MASK_MAJOR_VIDEOITEM:
			size += _CdsToDidl_Helper_GetSizeForVideoItem(fnEscapeLength, mediaObj, filter, &printThese);
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

	switch (mediaObj->MediaClass & CDS_CLASS_MASK_MINOR1)
	{
		case CDS_CLASS_MASK_MINOR1_MUSICALBUM:
			size += _CdsToDidl_Helper_GetSizeForMusicAlbum(fnEscapeLength, mediaObj, filter, &printThese);
			break;
		case CDS_CLASS_MASK_MINOR1_PHOTO:
			size += _CdsToDidl_Helper_GetSizeForPhoto(fnEscapeLength, mediaObj, filter, &printThese);
			break;
		case CDS_CLASS_MASK_MINOR1_AUDIOBROADCAST:
			size += _CdsToDidl_Helper_GetSizeForAudioBroadcast(fnEscapeLength, mediaObj, filter, &printThese);
			break;
		case CDS_CLASS_MASK_MINOR1_VIDEOBROADCAST:
			size += _CdsToDidl_Helper_GetSizeForVideoBroadcast(fnEscapeLength, mediaObj, filter, &printThese);
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

	/* validate the media class */
	if ((mediaObj->MediaClass != 0) && (size > 0))
	{
		imc_Object = (mediaObj->MediaClass & CDS_CLASS_MASK_OBJECT_TYPE) >> CDS_SHIFT_OBJECT_TYPE;
		imc_Major  = (mediaObj->MediaClass & CDS_CLASS_MASK_MAJOR) >> CDS_SHIFT_MAJOR_TYPE;
		imc_Minor1 = (mediaObj->MediaClass & CDS_CLASS_MASK_MINOR1) >> CDS_SHIFT_MINOR1_TYPE;
		imc_Minor2 = (mediaObj->MediaClass & CDS_CLASS_MASK_MINOR2) >> CDS_SHIFT_MINOR2_TYPE;

		/* check for various error conditions with media class */
		if ((imc_Object >= CDS_CLASS_OBJECT_TYPE_LEN) || (imc_Object < 1))
		{
			size = Error_CdsToDidl_UndefinedObjectType;
		}
		else if ((imc_Major >= CDS_CLASS_MAJOR_TYPE_LEN) || (imc_Major < 0))
		{
			size = Error_CdsToDidl_UndefinedMajorType;
		}
		else if	((imc_Minor1 >= CDS_CLASS_MINOR1_TYPE_LEN) || (imc_Minor1 < 0))
		{
			size = Error_CdsToDidl_UndefinedMinor1Type;
		}
		else if	((imc_Minor2 >= CDS_CLASS_MINOR2_TYPE_LEN) || (imc_Minor2 < 0))
		{

			/*
				*	TODO: If you want to return an error for an invalid
				*	index into CDS_CLASS_MINOR2_TYPE[], then use this
				*	code instead.
				*
				*	size = Error_CdsToDidl_UndefinedMinor2Type;
				*/

			/* ignore the MINOR2 field designator if it's out of range */
			imc_Minor2 = 0;
		}
		else
		{
			/* media class is valid - calculate length - assume no strings need escaping */
			size += (int) strlen(CDS_CLASS_OBJECT_TYPE[imc_Object]);
			size += (int) strlen(CDS_CLASS_MAJOR_TYPE[imc_Major]);
			size += (int) strlen(CDS_CLASS_MINOR1_TYPE[imc_Minor1]);
			size += (int) strlen(CDS_CLASS_MINOR2_TYPE[imc_Minor2]);
			size += CDS_DIDL_CLASS_ESCAPED_LEN + 4;	/*add 4 for additional . chars */

			/*
			*	Note the length needed for item/container element open and close tags.
			*	Also note additional length for restricted, searchable attributes.
			*/
			size++; /* restricted */
			if ((mediaObj->MediaClass & CDS_CLASS_MASK_CONTAINER) != 0)
			{
				size++; /* byte for searchable flag */
				size +=10; /* bytes for childCount attribute */
				size += CDS_DIDL_CONTAINER_START_ESCAPED_LEN;
				size += CDS_DIDL_CONTAINER_END_ESCAPED_LEN;

				/* containertype */
				if(mediaObj->TypeObject.Container.DlnaContainerType != 0)
				{
					size += CDS_DIDL_CONTAINERTYPE_ESCAPED_LEN;
					printThese |= CdsFilter_DlnaContainerType;
				}

				/* upnp:createClass */

				createClass = mediaObj->TypeObject.Container.CreateClass;

				while(
						(createClass != NULL) &&
						(createClass->MediaClass != 0) &&
						(size > 0)
						)
				{
					imc_Object = (createClass->MediaClass & CDS_CLASS_MASK_OBJECT_TYPE) >> CDS_SHIFT_OBJECT_TYPE;
					imc_Major  = (createClass->MediaClass & CDS_CLASS_MASK_MAJOR) >> CDS_SHIFT_MAJOR_TYPE;
					imc_Minor1 = (createClass->MediaClass & CDS_CLASS_MASK_MINOR1) >> CDS_SHIFT_MINOR1_TYPE;
					imc_Minor2 = (createClass->MediaClass & CDS_CLASS_MASK_MINOR2) >> CDS_SHIFT_MINOR2_TYPE;

					/* check for various error conditions with media class */
					if ((imc_Object >= CDS_CLASS_OBJECT_TYPE_LEN) || (imc_Object < 1))
					{
						size = Error_CdsToDidl_UndefinedObjectType;
					}
					else if ((imc_Major >= CDS_CLASS_MAJOR_TYPE_LEN) || (imc_Major < 0))
					{
						size = Error_CdsToDidl_UndefinedMajorType;
					}
					else if	((imc_Minor1 >= CDS_CLASS_MINOR1_TYPE_LEN) || (imc_Minor1 < 0))
					{
						size = Error_CdsToDidl_UndefinedMinor1Type;
					}
					else if	((imc_Minor2 >= CDS_CLASS_MINOR2_TYPE_LEN) || (imc_Minor2 < 0))
					{

						/*
						*	TODO: If you want to return an error for an invalid
						*	index into CDS_CLASS_MINOR2_TYPE[], then use this
						*	code instead.
						*
						*	size = Error_CdsToDidl_UndefinedMinor2Type;
						*/

						/* ignore the MINOR2 field designator if it's out of range */

						imc_Minor2 = 0;
					}
					else
					{
						/* media class is valid - calculate length - assume no strings need escaping */
						size += CDS_DIDL_CONTAINER_CREATECLASS_ESCAPED_LEN;
						size += 1;	/* 1 byte for '1' or '0' value for includeDerived*/
						size += (int) strlen(CDS_CLASS_OBJECT_TYPE[imc_Object]);
						size += (int) strlen(CDS_CLASS_MAJOR_TYPE[imc_Major]);
						size += (int) strlen(CDS_CLASS_MINOR1_TYPE[imc_Minor1]);
						size += (int) strlen(CDS_CLASS_MINOR2_TYPE[imc_Minor2]);
						size +=  4;	/* add 4 for additional . chars */
					}
					createClass = createClass->Next;
				}

				/* upnp:searchClass */

				searchClass = mediaObj->TypeObject.Container.SearchClass;

				while(
						(searchClass != NULL) &&
						(searchClass->MediaClass != 0) &&
						(size > 0)
						)
				{
					imc_Object = (searchClass->MediaClass & CDS_CLASS_MASK_OBJECT_TYPE) >> CDS_SHIFT_OBJECT_TYPE;
					imc_Major  = (searchClass->MediaClass & CDS_CLASS_MASK_MAJOR) >> CDS_SHIFT_MAJOR_TYPE;
					imc_Minor1 = (searchClass->MediaClass & CDS_CLASS_MASK_MINOR1) >> CDS_SHIFT_MINOR1_TYPE;
					imc_Minor2 = (searchClass->MediaClass & CDS_CLASS_MASK_MINOR2) >> CDS_SHIFT_MINOR2_TYPE;

					/* check for various error conditions with media class */
					if ((imc_Object >= CDS_CLASS_OBJECT_TYPE_LEN) || (imc_Object < 1))
					{
						size = Error_CdsToDidl_UndefinedObjectType;
					}
					else if ((imc_Major >= CDS_CLASS_MAJOR_TYPE_LEN) || (imc_Major < 0))
					{
						size = Error_CdsToDidl_UndefinedMajorType;
					}
					else if	((imc_Minor1 >= CDS_CLASS_MINOR1_TYPE_LEN) || (imc_Minor1 < 0))
					{
						size = Error_CdsToDidl_UndefinedMinor1Type;
					}
					else if	((imc_Minor2 >= CDS_CLASS_MINOR2_TYPE_LEN) || (imc_Minor2 < 0))
					{

						/*
						*	TODO: If you want to return an error for an invalid
						*	index into CDS_CLASS_MINOR2_TYPE[], then use this
						*	code instead.
						*
						*	size = Error_CdsToDidl_UndefinedMinor2Type;
						*/

						/* ignore the MINOR2 field designator if it's out of range */

						imc_Minor2 = 0;
					}
					else
					{
						/* media class is valid - calculate length - assume no strings need escaping */
						size += CDS_DIDL_CONTAINER_SEARCHCLASS_ESCAPED_LEN;
						size += 5;	/* 5 bytes for 'true' or 'false' value */
						size += (int) strlen(CDS_CLASS_OBJECT_TYPE[imc_Object]);
						size += (int) strlen(CDS_CLASS_MAJOR_TYPE[imc_Major]);
						size += (int) strlen(CDS_CLASS_MINOR1_TYPE[imc_Minor1]);
						size += (int) strlen(CDS_CLASS_MINOR2_TYPE[imc_Minor2]);
						size += 4;	/* add 4 for additional . chars */
					}
					searchClass = searchClass->Next;
				}
			}
			else if ((mediaObj->MediaClass & CDS_CLASS_MASK_ITEM) != 0)
			{
				size += CDS_DIDL_ITEM_START_ESCAPED_LEN;
				size += CDS_DIDL_ITEM_END_ESCAPED_LEN;
			}

			if((mediaObj->DlnaManaged != 0) && (filter & CdsFilter_DlnaManaged))
			{
				size += CDS_DIDL_DLNAMANAGED_ESCAPED_LEN + SIZE_INT32_AS_CHARS;
				printThese |= CdsFilter_DlnaManaged;
			}

			if(mediaObj->NumTakeOutGroups > 0 && mediaObj->TakeOutGroups != NULL)
			{
				size += _CdsToDidl_Helper_GetSizeForTakeOutGroups(fnEscapeLength, mediaObj, filter, &printThese);
			}

			/*
				*	At this point, we add the amount of memory needed
				*	for the resource elements to the size... if and only
				*	if <res> elements were requsted.
				*/
			if (filter & CdsFilter_Res)
			{
				printThese |= CdsFilter_Res;

				res = mediaObj->Res;
				while (res != NULL)
				{
					/* add size for minimal <res> element and its metadata */
					size += (CDS_DIDL_RES_ESCAPED_LEN + CDS_DIDL_RES_VALUE_ESCAPED_LEN);

					if (res->ProtocolInfo!= NULL)
					{
						size += fnEscapeLength(res->ProtocolInfo);
					}

					if (res->Value != NULL)
					{
						size += fnEscapeLength(res->Value);
					}

					if ((res->Bitrate >= 0) && (filter & CdsFilter_ResBitrate))
					{
						printThese |= CdsFilter_ResBitrate;
						size += (CDS_DIDL_RES_ATTRIB_BITRATE_ESCAPED_LEN + SIZE_INT32_AS_CHARS);
					}
					if ((res->BitsPerSample >= 0) && (filter & CdsFilter_ResBitsPerSample))
					{
						printThese |= CdsFilter_ResBitsPerSample;
						size += (CDS_DIDL_RES_ATTRIB_BITSPERSAMPLE_ESCAPED_LEN + SIZE_INT32_AS_CHARS);
					}
					if ((res->ColorDepth >= 0) && (filter & CdsFilter_ResColorDepth))
					{
						printThese |= CdsFilter_ResColorDepth;
						size += (CDS_DIDL_RES_ATTRIB_COLORDEPTH_ESCAPED_LEN	 + SIZE_INT32_AS_CHARS);
					}
					if ((res->Duration >= 0) && (filter & CdsFilter_ResDuration))
					{
						printThese |= CdsFilter_ResDuration;
						size += (CDS_DIDL_RES_ATTRIB_DURATION_ESCAPED_LEN + MAX_TIME_STRING_SIZE);
					}
					if ((res->NrAudioChannels >= 0) && (filter & CdsFilter_ResNrAudioChannels))
					{
						printThese |= CdsFilter_ResNrAudioChannels;
						size += (CDS_DIDL_RES_ATTRIB_NRAUDIOCHANNELS_ESCAPED_LEN + SIZE_INT32_AS_CHARS);
					}
					if ((res->Protection != NULL) && (filter & CdsFilter_ResProtection))
					{
						printThese |= CdsFilter_ResProtection;
						size += (CDS_DIDL_RES_ATTRIB_PROTECTION_ESCAPED_LEN + fnEscapeLength(res->Protection));
					}
					if ((res->ResolutionX >= 0) && (res->ResolutionY > 0) && (filter & CdsFilter_ResResolution))
					{
						printThese |= CdsFilter_ResResolution;
						size += (CDS_DIDL_RES_ATTRIB_RESOLUTION_ESCAPED_LEN + SIZE_INT32_AS_CHARS + SIZE_INT32_AS_CHARS);
					}
					if ((res->SampleFrequency >= 0) && (filter & CdsFilter_ResSampleFrequency))
					{
						printThese |= CdsFilter_ResSampleFrequency;
						size += (CDS_DIDL_RES_ATTRIB_SAMPLEFREQUENCY_ESCAPED_LEN + SIZE_INT32_AS_CHARS);
					}
					if ((res->Size >= 0) && (filter & CdsFilter_ResSize))
					{
						printThese |= CdsFilter_ResSize;
						size += (CDS_DIDL_RES_ATTRIB_SIZE_ESCAPED_LEN + SIZE_INT32_AS_CHARS);
					}
					if ((res->ImportUri != NULL) && (filter & CdsFilter_ResImportUri))
					{
						printThese |= CdsFilter_ResImportUri;
						size += (CDS_DIDL_RES_ATTRIB_IMPORTURI_ESCAPED_LEN + fnEscapeLength(res->ImportUri));
					}
					if ((res->IfoFileUri != NULL) && (filter & CdsFilter_ResIfoFileUri))
					{
						printThese |= CdsFilter_ResIfoFileUri;
						size += (CDS_DIDL_RES_ATTRIB_IFOFILEURI_ESCAPED_LEN + fnEscapeLength(res->IfoFileUri));
					}
					if ((res->ImportIfoFileUri != NULL) && (filter & CdsFilter_ResImportIfoFileUri))
					{
						printThese |= CdsFilter_ResImportIfoFileUri;
						size += (CDS_DIDL_RES_ATTRIB_IMPORTIFOFILEURI_ESCAPED_LEN + fnEscapeLength(res->ImportIfoFileUri));
					}
					if ((res->TrackTotal >= 0) && (filter & CdsFilter_ResTrackTotal))
					{
						printThese |= CdsFilter_ResTrackTotal;
						size += (CDS_DIDL_RES_ATTRIB_TRACKTOTAL_ESCAPED_LEN + SIZE_INT32_AS_CHARS);
					}
					if (((res->ResumeUpload == 1) || (res->ResumeUpload == 0)) && (filter & CdsFilter_ResResumeUpload))
					{
						printThese |= CdsFilter_ResResumeUpload;
						size += (CDS_DIDL_RES_ATTRIB_RESUMEUPLOAD_ESCAPED_LEN + SIZE_INT32_AS_CHARS);
					}
					if ((res->UploadedSize >= 0) && (filter & CdsFilter_ResUploadedSize))
					{
						printThese |= CdsFilter_ResUploadedSize;
						size += (CDS_DIDL_RES_ATTRIB_UPLOADEDSIZE_ESCAPED_LEN + SIZE_INT32_AS_CHARS);
					}

					res = res->Next;
				}
			}

			if (includeHeaderFooter != 0)
			{
				/* appropriately include length for header/footer, if requested */
				size += CDS_DIDL_HEADER_ESCAPED_LEN;
				size += CDS_DIDL_FOOTER_ESCAPED_LEN;
			}
		}
	}
	else
	{
		/* invalid media class - this is an error */
		size = 	Error_CdsToDidl_InvalidMediaClass;
	}

	if (size > 0)
	{
		/*
		 *	If this code executes, then the media object can be serialized to DIDL-Lite
		 *	without any problems.
		 */

		cp = retVal = (char*) malloc (size);

		if (cp == NULL)
		{
			fprintf(stderr, "!!! malloc failed in CdsToDidl_GetmediaObjectDidlEscaped() !!!\r\n");
		}

		/* print DIDL-Lite element if requested */
		if (includeHeaderFooter != 0)
		{
			cp += sprintf(cp, CDS_DIDL_HEADER_ESCAPED);
		}

		/* print <item> or <container> start */
		if ((mediaObj->MediaClass & CDS_CLASS_MASK_CONTAINER) != 0)
		{
			cp += sprintf(cp, CDS_DIDL_CONTAINER_START1_ESCAPED);

			if(mediaObj->ID != NULL)
			{
				cp += sprintf(cp, CDS_DIDL_CONTAINER_START2_ESCAPED);
				cp += fnEscape(cp, mediaObj->ID);
				cp += sprintf(cp, CDS_DIDL_DOUBLE_QUOTE_ESCAPED);
			}

			if(mediaObj->ParentID != NULL)
			{
				cp += sprintf(cp, CDS_DIDL_CONTAINER_START3_ESCAPED);
				cp += fnEscape(cp, mediaObj->ParentID);
				cp += sprintf(cp, CDS_DIDL_DOUBLE_QUOTE_ESCAPED);
			}
			
			cp += sprintf(cp, CDS_DIDL_CONTAINER_START4_ESCAPED);
			if (mediaObj->Flags & CDS_OBJPROP_FLAGS_Restricted)
			{
				cp[0] = '1';
			}
			else
			{
				cp[0] = '0';
			}
			cp++;
			cp += sprintf(cp, CDS_DIDL_DOUBLE_QUOTE_ESCAPED);
			
			/* compare directly against filter because printThese will never set CdsFilter_Searchable */
			if (filter & CdsFilter_Searchable)
			{
				cp += sprintf(cp, CDS_DIDL_CONTAINER_START5_ESCAPED);
				if (mediaObj->Flags & CDS_OBJPROP_FLAGS_Searchable )
				{
					cp[0] = '1';
				}
				else
				{
					cp[0] = '0';
				}
				cp++;
				cp += sprintf(cp, CDS_DIDL_DOUBLE_QUOTE_ESCAPED);
			}

			/* compare directly against filter because printThese will never set CdsFilter_Searchable */
			if (filter & CdsFilter_ChildCount)
			{
				cp += sprintf(cp, CDS_DIDL_CONTAINER_START6_ESCAPED, mediaObj->TypeObject.Container.ChildCount);
				cp += sprintf(cp, CDS_DIDL_DOUBLE_QUOTE_ESCAPED);
			}

			/* print dlna:dlnaManaged */
			if((printThese & CdsFilter_DlnaManaged) && (mediaObj->DlnaManaged != 0))
			{
				cp += sprintf(cp, CDS_DIDL_DLNAMANAGED_ESCAPED);
				cp += sprintf(cp, "%x", mediaObj->DlnaManaged);
				cp += sprintf(cp, CDS_DIDL_DOUBLE_QUOTE_ESCAPED);
			}

			cp += sprintf(cp, CDS_DIDL_CONTAINER_START7_ESCAPED);
		}
		else
		{
			cp += sprintf(cp, CDS_DIDL_ITEM_START1_ESCAPED);

			if(mediaObj->ID != NULL)
			{
				cp += sprintf(cp, CDS_DIDL_ITEM_START2_ESCAPED);
				cp += fnEscape(cp, mediaObj->ID);
				cp += sprintf(cp, CDS_DIDL_DOUBLE_QUOTE_ESCAPED);
			}

			if(mediaObj->ParentID != NULL)
			{
				cp += sprintf(cp, CDS_DIDL_ITEM_START3_ESCAPED);
				cp += fnEscape(cp, mediaObj->ParentID);
				cp += sprintf(cp, CDS_DIDL_DOUBLE_QUOTE_ESCAPED);
			}

			cp += sprintf(cp, CDS_DIDL_ITEM_START4_ESCAPED);
			if (mediaObj->Flags & CDS_OBJPROP_FLAGS_Restricted)
			{
				cp[0] = '1';
			}
			else
			{
				cp[0] = '0';
			}
			cp++;
			cp += sprintf(cp, CDS_DIDL_DOUBLE_QUOTE_ESCAPED);

			if (
				(mediaObj->TypeObject.Item.RefID != NULL) && (strlen(mediaObj->TypeObject.Item.RefID) > 0)
				)
			{
				cp += sprintf(cp, CDS_DIDL_ITEM_START5_ESCAPED);
				cp += fnEscape(cp, mediaObj->TypeObject.Item.RefID);
				cp += sprintf(cp, CDS_DIDL_DOUBLE_QUOTE_ESCAPED);
			}

			/* print dlna:dlnaManaged */
			if((printThese & CdsFilter_DlnaManaged) && (mediaObj->DlnaManaged != 0))
			{
				cp += sprintf(cp, CDS_DIDL_DLNAMANAGED_ESCAPED);
				cp += sprintf(cp, "%x", mediaObj->DlnaManaged);
				cp += sprintf(cp, CDS_DIDL_DOUBLE_QUOTE_ESCAPED);
			}
			cp += sprintf(cp, CDS_DIDL_ITEM_START6_ESCAPED);
		}

		/* print title */
		cp += sprintf(cp, CDS_DIDL_TITLE1_ESCAPED);
		cp += fnEscape(cp, mediaObj->Title);
		cp += sprintf(cp, CDS_DIDL_TITLE2_ESCAPED);

		/* print media class */
		cp += sprintf(cp, CDS_DIDL_CLASS1_ESCAPED);

		imc_Object = (mediaObj->MediaClass & CDS_CLASS_MASK_OBJECT_TYPE) >> CDS_SHIFT_OBJECT_TYPE;
		imc_Major  = (mediaObj->MediaClass & CDS_CLASS_MASK_MAJOR) >> CDS_SHIFT_MAJOR_TYPE;
		imc_Minor1 = (mediaObj->MediaClass & CDS_CLASS_MASK_MINOR1) >> CDS_SHIFT_MINOR1_TYPE;
		imc_Minor2 = (mediaObj->MediaClass & CDS_CLASS_MASK_MINOR2) >> CDS_SHIFT_MINOR2_TYPE;

		cp += sprintf(cp, CDS_CLASS_OBJECT_TYPE[imc_Object]);
		if (imc_Major > 0) { cp += sprintf(cp, ".%s", CDS_CLASS_MAJOR_TYPE[imc_Major]); }
		if (imc_Minor1 > 0) { cp += sprintf(cp, ".%s", CDS_CLASS_MINOR1_TYPE[imc_Minor1]); }
		if (imc_Minor2 > 0) { cp += sprintf(cp, ".%s", CDS_CLASS_MINOR2_TYPE[imc_Minor2]); }
		cp += sprintf(cp, CDS_DIDL_CLASS2_ESCAPED);

		/* print upnp:createClass */
		createClass = mediaObj->TypeObject.Container.CreateClass;

		while(createClass != NULL)
		{
			if(createClass->IncludeDerived == 0)
			{
				cp += sprintf(cp, CDS_DIDL_CONTAINER_CREATECLASS1_ESCAPED, "0");
			}
			else
			{
				cp += sprintf(cp, CDS_DIDL_CONTAINER_CREATECLASS1_ESCAPED, "1");
			}

			imc_Object = (createClass->MediaClass & CDS_CLASS_MASK_OBJECT_TYPE) >> CDS_SHIFT_OBJECT_TYPE;
			imc_Major  = (createClass->MediaClass & CDS_CLASS_MASK_MAJOR) >> CDS_SHIFT_MAJOR_TYPE;
			imc_Minor1 = (createClass->MediaClass & CDS_CLASS_MASK_MINOR1) >> CDS_SHIFT_MINOR1_TYPE;
			imc_Minor2 = (createClass->MediaClass & CDS_CLASS_MASK_MINOR2) >> CDS_SHIFT_MINOR2_TYPE;

			cp += sprintf(cp, CDS_CLASS_OBJECT_TYPE[imc_Object]);
			if (imc_Major > 0) { cp += sprintf(cp, ".%s", CDS_CLASS_MAJOR_TYPE[imc_Major]); }
			if (imc_Minor1 > 0) { cp += sprintf(cp, ".%s", CDS_CLASS_MINOR1_TYPE[imc_Minor1]); }
			if (imc_Minor2 > 0) { cp += sprintf(cp, ".%s", CDS_CLASS_MINOR2_TYPE[imc_Minor2]); }
			cp += sprintf(cp, CDS_DIDL_CONTAINER_CREATECLASS2_ESCAPED);
			createClass = createClass->Next;
		}

		/* print upnp:searchClass */

		searchClass = mediaObj->TypeObject.Container.SearchClass;

		while(searchClass != NULL)
		{
			if(searchClass->IncludeDerived == 0)
			{
				cp += sprintf(cp, CDS_DIDL_CONTAINER_SEARCHCLASS1_ESCAPED, "0");
			}
			else
			{
				cp += sprintf(cp, CDS_DIDL_CONTAINER_SEARCHCLASS1_ESCAPED, "1");
			}

			imc_Object = (searchClass->MediaClass & CDS_CLASS_MASK_OBJECT_TYPE) >> CDS_SHIFT_OBJECT_TYPE;
			imc_Major  = (searchClass->MediaClass & CDS_CLASS_MASK_MAJOR) >> CDS_SHIFT_MAJOR_TYPE;
			imc_Minor1 = (searchClass->MediaClass & CDS_CLASS_MASK_MINOR1) >> CDS_SHIFT_MINOR1_TYPE;
			imc_Minor2 = (searchClass->MediaClass & CDS_CLASS_MASK_MINOR2) >> CDS_SHIFT_MINOR2_TYPE;

			cp += sprintf(cp, CDS_CLASS_OBJECT_TYPE[imc_Object]);
			if (imc_Major > 0) { cp += sprintf(cp, ".%s", CDS_CLASS_MAJOR_TYPE[imc_Major]); }
			if (imc_Minor1 > 0) { cp += sprintf(cp, ".%s", CDS_CLASS_MINOR1_TYPE[imc_Minor1]); }
			if (imc_Minor2 > 0) { cp += sprintf(cp, ".%s", CDS_CLASS_MINOR2_TYPE[imc_Minor2]); }
			cp += sprintf(cp, CDS_DIDL_CONTAINER_SEARCHCLASS2_ESCAPED);
			searchClass = searchClass->Next;
		}

		if((printThese & CdsFilter_DlnaContainerType) && (mediaObj->TypeObject.Container.DlnaContainerType != 0))
		{
			cp += sprintf(cp, "%s", CDS_DIDL_CONTAINERTYPE_ESCAPED);
		}

		if((printThese & CdsFilter_TakeOut) && (mediaObj->TakeOutGroups != NULL) && (mediaObj->NumTakeOutGroups>0))
		{
			for(i=0;i<mediaObj->NumTakeOutGroups;i++)
			{
				cp += sprintf(cp, CDS_DIDL_TAKEOUTGROUP_ESCAPED, mediaObj->TakeOutGroups[i]);
			}
		}

		/* print creator */
		if (printThese & CdsFilter_Creator)
		{
			cp += sprintf(cp, CDS_DIDL_CREATOR1_ESCAPED);
			cp += fnEscape(cp, mediaObj->Creator);
			cp += sprintf(cp, CDS_DIDL_CREATOR2_ESCAPED);
		}

		/* MUSTDO: Add code that will serialize the metadata */

		switch (mediaObj->MediaClass & CDS_CLASS_MASK_MAJOR)
		{
		case CDS_CLASS_MASK_MAJOR_AUDIOITEM:
			if(printThese & CdsFilter_Album)
			{
				for(i=0;i<mediaObj->TypeMajor.AudioItem.NumAlbums;i++)
				{
					cp += sprintf(cp, CDS_DIDL_ALBUM1_ESCAPED);
					cp += fnEscape(cp, mediaObj->TypeMajor.AudioItem.Albums[i]);
					cp += sprintf(cp, CDS_DIDL_ALBUM2_ESCAPED);
				}
			}
			if(printThese & CdsFilter_Genre)
			{
				for(i=0;i<mediaObj->TypeMajor.AudioItem.NumGenres;i++)
				{
					cp += sprintf(cp, CDS_DIDL_GENRE1_ESCAPED);
					cp += fnEscape(cp, mediaObj->TypeMajor.AudioItem.Genres[i]);
					cp += sprintf(cp, CDS_DIDL_GENRE2_ESCAPED);
				}
			}
			if(printThese & CdsFilter_Date && mediaObj->TypeMajor.AudioItem.Date > 0)
			{
				timeStr = ILibTime_Serialize((time_t) mediaObj->TypeMajor.AudioItem.Date);
				cp += sprintf(cp, CDS_DIDL_DATE1_ESCAPED);
				cp += sprintf(cp, "%s", timeStr);
				cp += sprintf(cp, CDS_DIDL_DATE2_ESCAPED);
				free(timeStr);
			}
			break;
		case CDS_CLASS_MASK_MAJOR_IMAGEITEM:
			if(printThese & CdsFilter_Date && mediaObj->TypeMajor.ImageItem.Date > 0)
			{
				timeStr = ILibTime_Serialize((time_t) mediaObj->TypeMajor.ImageItem.Date);
				cp += sprintf(cp, CDS_DIDL_DATE1_ESCAPED);
				cp += sprintf(cp, "%s", timeStr);
				cp += sprintf(cp, CDS_DIDL_DATE2_ESCAPED);
				free(timeStr);
			}
			break;
		case CDS_CLASS_MASK_MAJOR_VIDEOITEM:
			if(printThese & CdsFilter_Genre)
			{
				for(i=0;i<mediaObj->TypeMajor.VideoItem.NumGenres;i++)
				{
					cp += sprintf(cp, CDS_DIDL_GENRE1_ESCAPED);
					cp += fnEscape(cp, mediaObj->TypeMajor.VideoItem.Genres[i]);
					cp += sprintf(cp, CDS_DIDL_GENRE2_ESCAPED);
				}
			}
			if(printThese & CdsFilter_Date && mediaObj->TypeMajor.ImageItem.Date > 0)
			{
				timeStr = ILibTime_Serialize((time_t) mediaObj->TypeMajor.ImageItem.Date);
				cp += sprintf(cp, CDS_DIDL_DATE1_ESCAPED);
				cp += sprintf(cp, "%s", timeStr);
				cp += sprintf(cp, CDS_DIDL_DATE2_ESCAPED);
				free(timeStr);
			}
			break;

		/*	TODO: If you add more structs to union MajorTypes, 
				then add appropriate code here.

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

		switch (mediaObj->MediaClass & CDS_CLASS_MASK_MINOR1)
		{
		case CDS_CLASS_MASK_MINOR1_MUSICALBUM:
			if(printThese & CdsFilter_Genre)
			{
				for(i=0;i<mediaObj->TypeMinor1.MusicAlbum.NumGenres;i++)
				{
					cp += sprintf(cp, CDS_DIDL_GENRE1_ESCAPED);
					cp += fnEscape(cp, mediaObj->TypeMinor1.MusicAlbum.Genres[i]);
					cp += sprintf(cp, CDS_DIDL_GENRE2_ESCAPED);
				}
			}
			break;
		case CDS_CLASS_MASK_MINOR1_PHOTO:
			if(printThese & CdsFilter_Album)
			{
				for(i=0;i<mediaObj->TypeMinor1.Photo.NumAlbums;i++)
				{
					cp += sprintf(cp, CDS_DIDL_ALBUM1_ESCAPED);
					cp += fnEscape(cp, mediaObj->TypeMinor1.Photo.Albums[i]);
					cp += sprintf(cp, CDS_DIDL_ALBUM2_ESCAPED);
				}
			}
			break;
		case CDS_CLASS_MASK_MINOR1_AUDIOBROADCAST:
			if(printThese & CdsFilter_ChannelName)
			{
				cp += sprintf(cp, CDS_DIDL_CHANNELNAME1_ESCAPED);
				cp += fnEscape(cp, mediaObj->TypeMinor1.AudioBroadcast.ChannelName);
				cp += sprintf(cp, CDS_DIDL_CHANNELNAME2_ESCAPED);
			}
			if(printThese & CdsFilter_ChannelNr)
			{
				cp += sprintf(cp, CDS_DIDL_CHANNELNR_ESCAPED, mediaObj->TypeMinor1.AudioBroadcast.ChannelNr);
			}
			break;
		case CDS_CLASS_MASK_MINOR1_VIDEOBROADCAST:
			if(printThese & CdsFilter_ChannelName)
			{
				cp += sprintf(cp, CDS_DIDL_CHANNELNAME1_ESCAPED);
				cp += fnEscape(cp, mediaObj->TypeMinor1.VideoBroadcast.ChannelName);
				cp += sprintf(cp, CDS_DIDL_CHANNELNAME2_ESCAPED);
			}
			if(printThese & CdsFilter_ChannelNr)
			{
				cp += sprintf(cp, CDS_DIDL_CHANNELNR_ESCAPED, mediaObj->TypeMinor1.VideoBroadcast.ChannelNr);
			}
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

		/* print resource and appropriate fields */
		if (printThese & CdsFilter_Res)
		{
			res = mediaObj->Res;

			while (res != NULL)
			{
				cp += sprintf(cp, CDS_DIDL_RES1_ESCAPED);
				/*
				 *	Need to double escape because DIDL-Lite is XML and the DIDL-Lite is
				 *	going within a SOAP response message, which is also XML.
				 */

				if (res->ProtocolInfo != NULL)
				{
					cp += fnEscape(cp, res->ProtocolInfo);
				}
				cp += sprintf(cp, CDS_DIDL_RES2_ESCAPED);

				if ((res->Bitrate >= 0) && (printThese & CdsFilter_ResBitrate))
				{
					/* CDS spec is strange in that bitRate is actually supposed to have the byte rate */
					cp += sprintf(cp, CDS_DIDL_RES_ATTRIB_BITRATE_ESCAPED, (res->Bitrate / 8));
				}
				if ((res->BitsPerSample >= 0) && (printThese & CdsFilter_ResBitsPerSample))
				{
					cp += sprintf(cp, CDS_DIDL_RES_ATTRIB_BITSPERSAMPLE_ESCAPED, res->BitsPerSample);
				}
				if ((res->ColorDepth >= 0) && (printThese & CdsFilter_ResColorDepth))
				{
					cp += sprintf(cp, CDS_DIDL_RES_ATTRIB_COLORDEPTH_ESCAPED, res->ColorDepth);
				}
				if ((res->Duration >= 0) && (printThese & CdsFilter_ResDuration))
				{
					_CdsToDidl_Helper_WriteDurationString(durationStr, res->Duration);
					cp += sprintf(cp, CDS_DIDL_RES_ATTRIB_DURATION_ESCAPED, durationStr);
				}
				if ((res->NrAudioChannels >= 0) && (printThese & CdsFilter_ResNrAudioChannels))
				{
					cp += sprintf(cp, CDS_DIDL_RES_ATTRIB_NRAUDIOCHANNELS_ESCAPED, res->NrAudioChannels);
				}
				if ((res->Protection != NULL) && (printThese & CdsFilter_ResProtection))
				{
					cp += sprintf(cp, CDS_DIDL_RES_ATTRIB_PROTECTION_ESCAPED, res->Protection);
				}
				if ((res->ResolutionX >= 0) && (res->ResolutionY >= 0) && (printThese & CdsFilter_ResResolution))
				{
					cp += sprintf(cp, CDS_DIDL_RES_ATTRIB_RESOLUTION_ESCAPED, res->ResolutionX, res->ResolutionY);
				}
				if ((res->SampleFrequency >= 0) && (printThese & CdsFilter_ResSampleFrequency))
				{
					cp += sprintf(cp, CDS_DIDL_RES_ATTRIB_SAMPLEFREQUENCY_ESCAPED, res->SampleFrequency);
				}
				if ((res->Size >= 0) && (printThese & CdsFilter_ResSize))
				{
					cp += sprintf(cp, CDS_DIDL_RES_ATTRIB_SIZE_ESCAPED, res->Size);
				}
				if ((res->ImportUri !=NULL) && (printThese & CdsFilter_ResImportUri))
				{
					cp += sprintf(cp, CDS_DIDL_RES_ATTRIB_IMPORTURI1_ESCAPED);
					cp += fnEscape(cp, res->ImportUri);
					cp += sprintf(cp, CDS_DIDL_RES_ATTRIB_IMPORTURI2_ESCAPED);
				}		
				if ((res->ImportIfoFileUri !=NULL) && (printThese & CdsFilter_ResImportIfoFileUri))
				{
					cp += sprintf(cp, CDS_DIDL_RES_ATTRIB_IMPORTIFOFILEURI1_ESCAPED);
					cp += fnEscape(cp, res->ImportIfoFileUri);
					cp += sprintf(cp, CDS_DIDL_RES_ATTRIB_IMPORTIFOFILEURI2_ESCAPED);
				}
				if ((res->IfoFileUri !=NULL) && (printThese & CdsFilter_ResIfoFileUri))
				{
					cp += sprintf(cp, CDS_DIDL_RES_ATTRIB_IFOFILEURI1_ESCAPED);
					cp += fnEscape(cp, res->IfoFileUri);
					cp += sprintf(cp, CDS_DIDL_RES_ATTRIB_IFOFILEURI2_ESCAPED);
				}
				if ((res->TrackTotal>=0) && (printThese & CdsFilter_ResTrackTotal))
				{
					cp += sprintf(cp, CDS_DIDL_RES_ATTRIB_TRACKTOTAL_ESCAPED, res->TrackTotal);
				}
				if (((res->ResumeUpload==1)||(res->ResumeUpload==0)) && (printThese & CdsFilter_ResResumeUpload))
				{
					cp += sprintf(cp, CDS_DIDL_RES_ATTRIB_RESUMEUPLOAD_ESCAPED, res->ResumeUpload);
				}
				if ((res->UploadedSize>=0) && (printThese & CdsFilter_ResUploadedSize))
				{
					cp += sprintf(cp, CDS_DIDL_RES_ATTRIB_UPLOADEDSIZE_ESCAPED, res->UploadedSize);
				}
				cp += sprintf(cp, CDS_DIDL_RES_VALUE1_ESCAPED);
				if (res->Value != NULL)
				{
					cp += fnEscape(cp, res->Value);
				}
				cp += sprintf(cp, CDS_DIDL_RES_VALUE2_ESCAPED);

				res = res->Next;
			}
		}

		if ((mediaObj->MediaClass &  CDS_CLASS_MASK_CONTAINER) != 0)
		{
			cp += sprintf(cp, CDS_DIDL_CONTAINER_END_ESCAPED);
		}
		else
		{
			cp += sprintf(cp, CDS_DIDL_ITEM_END_ESCAPED);
		}

		if (includeHeaderFooter != 0)
		{
			cp += sprintf(cp, CDS_DIDL_FOOTER_ESCAPED);
		}

		len = (int) (cp - retVal);
		if (len >= size)
		{
			/* we overwrote memory - this is bad */
			free (retVal);
			retVal = NULL;
			size = Error_CdsToDidl_CorruptedMemory;
		}
		else
		{
			size = len;
			retVal[size] = '\0';
		}
	}

	if (outErrorOrDidlLen != NULL)
	{
		*outErrorOrDidlLen = size;
	}

	return retVal;
}

char* CDS_SerializeObjectToDidlUnescaped (struct CdsObject *cds_obj, int cds_obj_metadata_escaped, unsigned int filter, int include_header_footer, int *out_error_or_didl_len)
{
	int outputLen;
	char* retVal = CDS_SerializeObjectToDidl(cds_obj, cds_obj_metadata_escaped, filter, include_header_footer, &outputLen);

	/* Be sure to unescape it first. */
	ILibInPlaceXmlUnEscape(retVal);

	if (out_error_or_didl_len != NULL && strlen(retVal) > 0)
	{
		*out_error_or_didl_len = (int) strlen(retVal);
	}
	return retVal;
}

/*
 *	Copies bytes from copyFrom to copyHere.
 *	Will not copy more than copyMaxChars bytes.
 *	Stops copying when ., <, null, or " char is found.
 *  and skip the number of terminators if skipCount is > 0
 */
void _DidlToCds_Helper_CopyUntilClassFragmentTerminator(char *copyHere, const char *copyFrom, int copyMaxChars, int skipCount)
{
	int i;
	int count = 0;
	char c;
	
	for (i=0; i < copyMaxChars; i++)
	{
		c = copyFrom[i];
		
		if (c == '.' || c == '<' || c == '\0' || c == '"')
		{
			count++;
			if(count>skipCount)
			{
				copyHere[i] = '\0';
				return;
			}
			else
			{
				copyHere[i] = c;
			}
		}
		else
		{
			copyHere[i] = c;
		}
	}
	copyHere[i] = '\0';
}

/*
 *	Given an array of strings, finds the index in that array with a matching string.
 */
#ifdef _POSIX
#define stricmp strcasecmp
#endif
int _DidlToCds_Helper_FindStringInArray(const char* str,const char** strarray,const int strarraylen)
{
	int i;
	for (i=0;i<strarraylen;i++) {if (stricmp(str,strarray[i]) == 0) {return i;}}
	return -1;
}

int _DidlToCds_Helper_StrCharArraySize(const char** values, unsigned int numValues)
{
	int size = 0;
	unsigned int i;

	for (i = 0; i < numValues; i++)
	{
		/* use length of string */
		size += ((int) strlen(values[i]));
	}
	size++; /* for null terminator */

	return size;
}

/*
 *	Calculates the number of bytes needed to store the string data for the CDS object.
 *
 *		obj			: this is the CDS object that has data values
 *		obj2		: Deprecated. Use NULL. 
 *					: An older implementation of MSCP used a single shared memory block
 *					: for all CDS objects in a set of results. Unfortunately, this made it
 *					: impossible to individual CDS objects within a list. Developers had
 *					: to deallocate the entire list. By sending NULL, you force the method
 *					: to increment the size needed for data.
 *					:
 *					: By sending a non-NULL value for this param, you allow this method
 *					: to assign a field on 'obj' to point to the same data that the
 *					: corresponding field on 'obj2' is pointing to (assuming the data values
 *					: are the same).
 */
int _DidlToCds_Helper_GetRequiredSizeForMediaObject(const struct CdsObject *obj)
{
	int retVal;
	struct CdsResource *res;

	retVal = 0;

	/*
	 *	For most fields, the pattern is simple. 
	 *		-check for non-null on the desired field (on obj)
	 *		-if obj2 is specified and the fields on obj and obj2 match, then
	 *			assign obj's field will point to the same data that obj2 is
	 *			pointing to.
	 *		-else increment retVal by the size needed to store the field on obj.
	 *			Be sure to add 1 for NULL terminator.
	 */

	if (obj->ID != NULL)
	{
		retVal += ((int) strlen(obj->ID) +1);
	}

	if (obj->ParentID != NULL)
	{
		retVal += ((int) strlen(obj->ParentID) +1);
	}

	if (obj->Title != NULL)
	{
		retVal += ((int) strlen(obj->Title) +1);
	}

	if (obj->Creator != NULL)
	{
		retVal += ((int) strlen(obj->Creator) +1);

	}

	if(obj->TakeOutGroups != NULL && obj->NumTakeOutGroups>0)
	{
		retVal += _DidlToCds_Helper_StrCharArraySize((const char **)obj->TakeOutGroups, obj->NumTakeOutGroups);
	}

	switch (obj->MediaClass & CDS_CLASS_MASK_OBJECT_TYPE)
	{
		case CDS_CLASS_MASK_ITEM:
			if(obj->TypeObject.Item.RefID != NULL)
			{
				retVal += ((int) strlen(obj->TypeObject.Item.RefID) + 1);
			}
			break;

			/*	TODO: If containers ever have string data specific to them
				then you need to add appropriate code here.
				case CDS_CLASS_MASK_CONTAINER:
			break;
			*/
	}

	switch (obj->MediaClass & CDS_CLASS_MASK_MAJOR)
	{
		case CDS_CLASS_MASK_MAJOR_AUDIOITEM:
			if(obj->TypeMajor.AudioItem.Albums!=NULL && obj->TypeMajor.AudioItem.NumAlbums>0)
			{
				retVal += _DidlToCds_Helper_StrCharArraySize((const char **)obj->TypeMajor.AudioItem.Albums, obj->TypeMajor.AudioItem.NumAlbums);
			}
			if(obj->TypeMajor.AudioItem.Genres!=NULL && obj->TypeMajor.AudioItem.NumGenres>0)
			{
				retVal += _DidlToCds_Helper_StrCharArraySize((const char **)obj->TypeMajor.AudioItem.Genres, obj->TypeMajor.AudioItem.NumGenres);
			}
			break;
		case CDS_CLASS_MASK_MAJOR_IMAGEITEM:
			break;
		case CDS_CLASS_MASK_MAJOR_VIDEOITEM:
			if(obj->TypeMajor.VideoItem.Genres!=NULL && obj->TypeMajor.VideoItem.NumGenres>0)
			{
				retVal += _DidlToCds_Helper_StrCharArraySize((const char **)obj->TypeMajor.VideoItem.Genres, obj->TypeMajor.VideoItem.NumGenres);
			}
			break;

			/*	TODO: If you add more structs to union MajorTypes, 
				then add appropriate code here.

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

	switch (obj->MediaClass & CDS_CLASS_MASK_MINOR1)
	{
		case CDS_CLASS_MASK_MINOR1_MUSICALBUM:
			if(obj->TypeMinor1.MusicAlbum.Genres!=NULL && obj->TypeMinor1.MusicAlbum.NumGenres>0)
			{
				retVal += _DidlToCds_Helper_StrCharArraySize((const char **)obj->TypeMinor1.MusicAlbum.Genres, obj->TypeMinor1.MusicAlbum.NumGenres);
			}
			break;
		case CDS_CLASS_MASK_MINOR1_PHOTO:
			if(obj->TypeMinor1.Photo.Albums!=NULL && obj->TypeMinor1.Photo.NumAlbums>0)
			{
				retVal += _DidlToCds_Helper_StrCharArraySize((const char **)obj->TypeMinor1.Photo.Albums, obj->TypeMinor1.Photo.NumAlbums);
			}
			break;
		case CDS_CLASS_MASK_MINOR1_AUDIOBROADCAST:
			if(obj->TypeMinor1.AudioBroadcast.ChannelName!=NULL)
			{
				retVal += ((int) strlen(obj->TypeMinor1.AudioBroadcast.ChannelName) + 1);
			}
			break;
		case CDS_CLASS_MASK_MINOR1_VIDEOBROADCAST:
			if(obj->TypeMinor1.VideoBroadcast.ChannelName!=NULL)
			{
				retVal += ((int) strlen(obj->TypeMinor1.VideoBroadcast.ChannelName) + 1);
			}
			break;

			/*	TODO: If you add more structs to union Minor1Types, 
				then add appropriate code here.

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

	/*
	 *	The pattern for <res> elements is as follows.
	 *	-Enumerate through each <res> element
	 *		-determine if we need to allocate memory for each res attribute (i.e. non-NULL value)
	 *		-Enumerate through previous <res> elements in our list
	 *			-if the res attribute matches a previous value, 
	 *			 then we don't need to allocate additional memory for the field
	 *		-if we determine that we need to allocate additional memory, then increase retVal by the 
	 *		 necessary amount.
	 */

	res = obj->Res;

	while (res != NULL)
	{
		/* determine whether we need to store data for the attributes */
		if(res->ProtocolInfo != NULL)
		{
			retVal += ((int) strlen(res->ProtocolInfo) +1);
		}
		if (res->Value != NULL)
		{
			retVal += ((int) strlen(res->Value) +1);
		}
		if(res->Protection != NULL)
		{
			retVal += ((int) strlen(res->Protection) +1);
		}
		if(res->IfoFileUri != NULL)
		{
			retVal += ((int) strlen(res->IfoFileUri) +1);
		}
		if(res->ImportIfoFileUri != NULL)
		{
			retVal += ((int) strlen(res->ImportIfoFileUri) +1);
		}
		if(res->ImportUri != NULL)
		{
			retVal += ((int) strlen(res->ImportUri) +1);
		}
		res = res->Next;
	}

	return retVal;
}

void _DidlToCds_Helper_StringFixup(char **fixThis, char** di, char *emptyStr, const char *data, const char *rangeStart, const char *rangeEnd)
{
	int len;

	if (data != NULL)
	{
		if ((rangeStart <= data) && (data <= rangeEnd))
		{
			/* store an XML-unescaped representation */

			*fixThis = *di;
			len = (int) strlen(data);
			memcpy(*di, data, len);

			ILibInPlaceXmlUnEscape(*di);

			*di = *di + len + 1;
		}
		else
		{
			*fixThis = (char*)data;
		}
	}
	else
	{
		*fixThis = emptyStr;
	}
}

void _DidlToCds_Helper_AddToArray(char*** array, int num_strings, char* str, int str_len)
{
	char** newArray = NULL;
	int i;

	if(num_strings == 0)
	{
		/* array is empty, created new array with 1 element */
		*array = (char**) malloc(1 * sizeof(char*));
		memset(*array, 0, 1 * sizeof(char*));
		(*array)[0] = (char*) malloc(str_len + 1);
		strcpy((*array)[0], str);
	}
	else if(num_strings > 0 && *array!=NULL)
	{
		/* array has at least 1 element, make a new array with 1 more element, copy the old array to the new array */

		/* allocate a new array of to hold 1 additional element */
		newArray = (char**) malloc((num_strings + 1) * sizeof(char*));
		memset(newArray, 0, (num_strings + 1) * sizeof(char*));

		/* copy the old array into new array */
		for(i=0;i<num_strings;i++)
		{
			newArray[i] = (char*) malloc(strlen((*array)[i]) + 1);
			strcpy(newArray[i], (*array)[i]);
			free((*array)[i]);
		}
		free(*array);

		/* add the last str */
		newArray[i] = (char*) malloc(str_len + 1);
		strcpy(newArray[i], str);

		*array = newArray;
	}
}


/*
 *	Creates an CdsObject from an XML node representing a CDS object.
 *
 *		node				: the XML data that represents our CDS object
 *		attribs				: the XML attributes of the CDS object (i.e. item or container attributes)
 *		isItem				: nonzero indicates the CDS object is a CDS item
 *		rangeStart/rangeEnd : the actual string data for 'node' occupies the [rangeStart,rangeEnd] portion of memory.
 */
struct CdsObject* CDS_DeserializeDidlToObject(struct ILibXMLNode *node, struct ILibXMLAttribute *attribs, int isItem, const char *rangeStart, const char *rangeEnd)
{
	struct ILibXMLNode *startNode;
	struct ILibXMLAttribute *att;

	struct CdsObject* newObj;
	struct CdsResource **res = NULL;
	struct CdsCreateClass **createClass = NULL;
	struct CdsSearchClass **searchClass = NULL;

	char* prefixNS;
	char* innerXml;
	int innerXmlLen;
	char classFragment[CDS_MAX_CLASS_FRAGMENT_SIZE];
	int indexIntoArray;

	int dataSize;
	int mallocSize;

	int l;
	int i;

	#ifdef _DEBUG
	/* PRECONDITION: node is a start node*/
	if (node->StartTag == 0)
	{
		printf("CDS_DeserializeDidlToObject requires node->StartTag!=0.\r\n");
		ASSERT(0);
	}
	
	/* PRECONDITION: node->Name is this node is a container or item */
	if (!((node->NameLength == (int)strlen(CDS_TAG_CONTAINER) && strncmp(node->Name, CDS_TAG_CONTAINER, node->NameLength) == 0) ||
		  (node->NameLength == (int)strlen(CDS_TAG_ITEM) && strncmp(node->Name, CDS_TAG_ITEM, node->NameLength) == 0)))
	{
		printf("CDS_DeserializeDidlToObject requires item or container node.\r\n");
		ASSERT(0);
	}
	#endif

	/* initialize newObj; init flags appropriately */

	newObj = CDS_AllocateObject();

	newObj->Flags |= CDS_OBJPROP_FLAGS_Restricted;	/* assume object is restricted */
	if (isItem == 0)
	{
		newObj->Flags |= CDS_OBJPROP_FLAGS_Searchable;/* assume container is searchable */
	}

	/*
	 *
	 *	Parse the item/container node and set the pointers in tempObj
	 *	to point into the memory referenced by node.
	 *
	 */

	/* Parse the attributes of the item/container */
	att = attribs;
	while (att != NULL)
	{
		/* [DONOTREPARSE] null terminate name and value. */
		att->Name[att->NameLength] = '\0';
		att->Value[att->ValueLength] = '\0';

		/* we need to unescape media data */
		ILibInPlaceXmlUnEscape(att->Value);

		if (strcmp(att->Name, CDS_ATTRIB_ID) == 0)
		{
			newObj->ID = (char*) malloc(att->ValueLength + 1);
			strcpy(newObj->ID, att->Value);
			newObj->DeallocateThese |= CDS_ALLOC_ID;
		}
		else if (strcmp(att->Name, CDS_ATTRIB_PARENTID) == 0)
		{
			newObj->ParentID = (char*) malloc(att->ValueLength + 1);
			strcpy(newObj->ParentID, att->Value);
			newObj->DeallocateThese |= CDS_ALLOC_ParentID;
		}
		else if (strcmp(att->Name, CDS_ATTRIB_RESTRICTED) == 0)
		{
			if (_DidlToCds_Helper_FindStringInArray(att->Value, CDS_TRUE_STRINGS, CDS_TRUE_STRINGS_LEN) >= 0)
			{
				/* set the restricted flag. */
				newObj->Flags |= CDS_OBJPROP_FLAGS_Restricted;
			}
			else
			{
				newObj->Flags &= (~CDS_OBJPROP_FLAGS_Restricted);
			}
		}
		else if ((isItem == 0) && (strcmp(att->Name, CDS_ATTRIB_SEARCHABLE) == 0))
		{
			if (_DidlToCds_Helper_FindStringInArray(att->Value, CDS_TRUE_STRINGS, CDS_TRUE_STRINGS_LEN) >= 0)
			{
				/* set the searchable flag. */
				newObj->Flags |= CDS_OBJPROP_FLAGS_Searchable;
			}
			else
			{
				newObj->Flags &= (~CDS_OBJPROP_FLAGS_Searchable);
			}
		}
		else if ((isItem != 0) && (strcmp(att->Name, CDS_ATTRIB_REFID) == 0))
		{
			newObj->TypeObject.Item.RefID = (char*) malloc(att->ValueLength + 1);
			strcpy(newObj->TypeObject.Item.RefID, att->Value);
			newObj->DeallocateThese |= CDS_ALLOC_RefID;
		}
		else if (strcmp(att->Name, CDS_ATTRIB_DLNAMANAGED) == 0)
		{
			newObj->DlnaManaged = strtol (att->Value, NULL, 16);
		}
		att = att->Next;
	}

	/*
	 *
	 *	Iterate through the child nodes of the startNode
	 *	and set the title, creator, and resources for
	 *	the media object.
	 *
	 */

	startNode = node;
	node = startNode->Next;
	while (node != startNode->ClosingTag)
	{
		if (node->StartTag != 0)
		{
			/* [DONOTREPARSE] null terminate name */
			attribs = ILibGetXMLAttributes(node);
			att = attribs;
			node->Name[node->NameLength] = '\0';

			/* parsing CDS object... build <res> */
			if (strcmp(node->Name, CDS_TAG_RESOURCE) == 0)
			{
				/*
				 *
				 *	Create a new resource element and add it
				 *	to the existing list of resources for the
				 *	media object. The resource will point to 
				 *	memory in XML, but we'll change where they
				 *	point at the very end.
				 *
				 */

				/* res point to the first <res> element */
				res = &newObj->Res;

				/* if there's already element, enumerate to the end of the list */
				while((*res)!= NULL)
				{
					res = &(*res)->Next;
				}

				(*res) = (struct CdsResource*) CDS_AllocateResource();
				
				/* Extract the protocolInfo from the element */
				while (att != NULL)
				{
					/* [DONOTREPARSE] */
					att->Name[att->NameLength] = '\0';
					att->Value[att->ValueLength] = '\0';

					/* we need to unescape media data */
					ILibInPlaceXmlUnEscape(att->Value);
								
					if (strcmp(att->Name, CDS_ATTRIB_PROTOCOLINFO) == 0)
					{
						(*res)->ProtocolInfo = (char*) malloc(att->ValueLength + 1);
						strcpy((*res)->ProtocolInfo, att->Value);
						(*res)->Allocated |= CDS_RES_ALLOC_ProtocolInfo;
					}
					else if (strcmp(att->Name, CDS_ATTRIB_RESOLUTION) == 0)
					{
						// ToDo: support for resolution ?x?
						l = (int) strlen(att->Value);
						for(i =0;i<l;i++)
						{
							if(att->Value[i] == 'x' || att->Value[i] == 'X')
							{
								att->Value[l] = '\0';
								att->Value[i] = '\0';
//								c = att->Value[i];
								(*res)->ResolutionY = atoi(att->Value+i);
								(*res)->ResolutionX = atoi(att->Value);
							}
						}
					}
					else if (strcmp(att->Name, CDS_ATTRIB_DURATION) == 0)
					{
						(*res)->Duration = _CdsToDidl_Helper_ParseDurationString(att->Value);
					}
					else if (strcmp(att->Name, CDS_ATTRIB_BITRATE) == 0)
					{
						ILibGetLong(att->Value, att->ValueLength, (long *)&((*res)->Bitrate));
					}
					else if (strcmp(att->Name, CDS_ATTRIB_BITSPERSAMPLE) == 0)
					{
						ILibGetLong(att->Value, att->ValueLength, (long *)&((*res)->BitsPerSample));
					}
					else if (strcmp(att->Name, CDS_ATTRIB_COLORDEPTH) == 0)
					{
						ILibGetLong(att->Value, att->ValueLength, (long *)&((*res)->ColorDepth));
					}
					else if (strcmp(att->Name, CDS_ATTRIB_NRAUDIOCHANNELS) == 0)
					{
						ILibGetLong(att->Value, att->ValueLength, (long *)&((*res)->NrAudioChannels));
					}
					else if (strcmp(att->Name, CDS_ATTRIB_PROTECTION) == 0)
					{
						(*res)->Protection = (char*) malloc(att->ValueLength + 1);
						strcpy((*res)->Protection, att->Value);
						(*res)->Allocated |= CDS_RES_ALLOC_Protection;
					}
					else if (strcmp(att->Name, CDS_ATTRIB_SAMPLEFREQUENCY) == 0)
					{
						ILibGetLong(att->Value, att->ValueLength, (long *)&((*res)->SampleFrequency));
					}
					else if (strcmp(att->Name, CDS_ATTRIB_SIZE) == 0)
					{
						ILibGetLong(att->Value, att->ValueLength, (long *)&((*res)->Size));
					}

					else if (strcmp(att->Name, CDS_ATTRIB_IMPORTURI) == 0)
					{
						(*res)->ImportUri = (char*) malloc(att->ValueLength + 1);
						strcpy((*res)->ImportUri, att->Value);
						(*res)->Allocated |= CDS_RES_ALLOC_ImportUri;
					}
					else if (strcmp(att->Name, CDS_ATTRIB_IFOFILEURI) == 0)
					{
						(*res)->IfoFileUri = (char*) malloc(att->ValueLength + 1);
						strcpy((*res)->IfoFileUri, att->Value);
						(*res)->Allocated |= CDS_RES_ALLOC_IfoFileUri;
					}
					else if (strcmp(att->Name, CDS_ATTRIB_IMPORTIFOFILEURI) == 0)
					{
						(*res)->ImportIfoFileUri = (char*) malloc(att->ValueLength + 1);
						strcpy((*res)->ImportIfoFileUri, att->Value);
						(*res)->Allocated |= CDS_RES_ALLOC_ImportIfoFileUri;
					}
					else if (strcmp(att->Name, CDS_ATTRIB_RESUMEUPLOAD) == 0)
					{
						(*res)->ResumeUpload = atoi(att->Value);
					}
					else if (strcmp(att->Name, CDS_ATTRIB_UPLOADEDSIZE) == 0)
					{
						ILibGetLong(att->Value, att->ValueLength, (long *)&((*res)->UploadedSize));
					}
					else if (strcmp(att->Name, CDS_ATTRIB_TRACKTOTAL) == 0)
					{
						ILibGetLong(att->Value, att->ValueLength, (long *)&((*res)->TrackTotal));
					}
										
					att = att->Next;
				}

				/* grab the URI */

				innerXmlLen = ILibReadInnerXML(node, &innerXml);
				innerXml[innerXmlLen] = '\0';
				/* we need to unescape media data */
				innerXmlLen = ILibInPlaceXmlUnEscape(innerXml);

				if(innerXmlLen>0)
				{
					(*res)->Value = (char*) malloc(innerXmlLen + 1);
					strcpy((*res)->Value, innerXml);
					(*res)->Allocated |= CDS_RES_ALLOC_Value;
				}
			}

			/* parsing CDS object... build simple string tags */
			else if (strcmp(node->Name, CDS_TAG_TITLE) == 0)
			{
				prefixNS = ILibXML_LookupNamespace(node,node->NSTag,node->NSLength);
				if(strncmp(prefixNS, CDS_XML_NAMESPACE_DC, CDS_XML_NAMESPACE_DC_LEN)==0)
				{
					innerXmlLen = ILibReadInnerXML(node, &innerXml);
					innerXml[innerXmlLen] = '\0';
					/* we need to unescape media data */
					innerXmlLen = ILibInPlaceXmlUnEscape(innerXml);

					newObj->Title = (char*) malloc(innerXmlLen + 1);
					strcpy(newObj->Title, innerXml);
					newObj->DeallocateThese |= CDS_ALLOC_Title;
				}
			}
			else if (strcmp(node->Name, CDS_TAG_CREATOR) == 0)
			{
				prefixNS = ILibXML_LookupNamespace(node,node->NSTag,node->NSLength);
				if(strncmp(prefixNS, CDS_XML_NAMESPACE_DC, CDS_XML_NAMESPACE_DC_LEN)==0)
				{

					innerXmlLen = ILibReadInnerXML(node, &innerXml);
					innerXml[innerXmlLen] = '\0';
					/* we need to unescape media data */
					innerXmlLen = ILibInPlaceXmlUnEscape(innerXml);

					newObj->Creator = (char*) malloc(innerXmlLen + 1);
					strcpy(newObj->Creator, innerXml);
					newObj->DeallocateThese |= CDS_ALLOC_Creator;
				}
			}
			else if (strcmp(node->Name, CDS_TAG_DATE) == 0)
			{
				prefixNS = ILibXML_LookupNamespace(node,node->NSTag,node->NSLength);
				if(strncmp(prefixNS, CDS_XML_NAMESPACE_DC, CDS_XML_NAMESPACE_DC_LEN)==0)
				{
					innerXmlLen = ILibReadInnerXML(node, &innerXml);
					innerXml[innerXmlLen] = '\0';
					switch (newObj->MediaClass & CDS_CLASS_MASK_MAJOR)
					{
						case CDS_CLASS_MASK_MAJOR_AUDIOITEM:
							newObj->TypeMajor.AudioItem.Date = (long) ILibTime_Parse(innerXml);
							break;
						case CDS_CLASS_MASK_MAJOR_IMAGEITEM:
							newObj->TypeMajor.ImageItem.Date = (long) ILibTime_Parse(innerXml);
							break;
						case CDS_CLASS_MASK_MAJOR_VIDEOITEM:
							newObj->TypeMajor.VideoItem.Date = (long) ILibTime_Parse(innerXml);
							break;
					}
					/*
						TODO: If you add more structs to union MajorTypes, MinorTypes1, MinorTypes2, 
						then add appropriate code here.
					*/
				}
			}
			/* parsing CDS object... build <upnp:class> */
			else if (strcmp(node->Name, CDS_TAG_MEDIACLASS) == 0)
			{
				prefixNS = ILibXML_LookupNamespace(node,node->NSTag,node->NSLength);
				if(strncmp(prefixNS, CDS_XML_NAMESPACE_UPNP, CDS_XML_NAMESPACE_UPNP_LEN)==0)
				{
					/* Figure out proper enum value given the specified media class */
					innerXmlLen = ILibReadInnerXML(node, &innerXml);

					/* initialize to bad class */
					newObj->MediaClass = CDS_CLASS_MASK_BADCLASS;
								
					/* determine object type */
					_DidlToCds_Helper_CopyUntilClassFragmentTerminator(classFragment, innerXml, MIN(innerXmlLen, CDS_MAX_CLASS_FRAGMENT_LEN), 1);
					indexIntoArray = _DidlToCds_Helper_FindStringInArray(classFragment, CDS_CLASS_OBJECT_TYPE, CDS_CLASS_OBJECT_TYPE_LEN);

					if (indexIntoArray > 0)
					{
						innerXml += ((int) strlen(CDS_CLASS_OBJECT_TYPE[indexIntoArray]) + 1);
						newObj->MediaClass |= (indexIntoArray << CDS_SHIFT_OBJECT_TYPE);
									
						/* Determine major type */
						_DidlToCds_Helper_CopyUntilClassFragmentTerminator(classFragment, innerXml, MIN(innerXmlLen, CDS_MAX_CLASS_FRAGMENT_LEN), 0);
						indexIntoArray = _DidlToCds_Helper_FindStringInArray(classFragment, CDS_CLASS_MAJOR_TYPE, CDS_CLASS_MAJOR_TYPE_LEN);
						if (indexIntoArray > 0)
						{
							innerXml += ((int) strlen(CDS_CLASS_MAJOR_TYPE[indexIntoArray]) + 1);
							newObj->MediaClass |= (indexIntoArray << CDS_SHIFT_MAJOR_TYPE);

							/* Determine minor type */
							_DidlToCds_Helper_CopyUntilClassFragmentTerminator(classFragment, innerXml, MIN(innerXmlLen, CDS_MAX_CLASS_FRAGMENT_LEN), 0);
							indexIntoArray = _DidlToCds_Helper_FindStringInArray(classFragment, CDS_CLASS_MAJOR_TYPE, CDS_CLASS_MAJOR_TYPE_LEN);
							if (indexIntoArray > 0)
							{
								newObj->MediaClass |= (indexIntoArray << CDS_SHIFT_MINOR1_TYPE);
								/* TODO : Add vendor-specific supported minor types parsing here */
							}
						}
					}
				}
			}
			else if (strcmp(node->Name, CDS_TAG_CREATECLASS) == 0)
			{
				prefixNS = ILibXML_LookupNamespace(node,node->NSTag,node->NSLength);
				if(strncmp(prefixNS, CDS_XML_NAMESPACE_UPNP, CDS_XML_NAMESPACE_UPNP_LEN)==0)
				{
					if((newObj->MediaClass & CDS_CLASS_MASK_OBJECT_TYPE) == CDS_CLASS_MASK_CONTAINER)
					{

						createClass = &newObj->TypeObject.Container.CreateClass;					

						while((*createClass)!=NULL)
						{
							createClass = &(*createClass)->Next;
						}

						(*createClass) = (struct CdsCreateClass*) malloc(sizeof(struct CdsCreateClass));
						memset((*createClass), 0, sizeof(struct CdsCreateClass));

						while (att != NULL)
						{
							att->Name[att->NameLength] = '\0';
							att->Value[att->ValueLength] = '\0';

							if (strcmp(att->Name, CDS_ATTRIB_INCLUDEDERIVED) == 0)
							{
								if (_DidlToCds_Helper_FindStringInArray(att->Value, CDS_TRUE_STRINGS, CDS_TRUE_STRINGS_LEN) >= 0)
								{
									/* set the includeDerived flag. */
									(*createClass)->IncludeDerived = 1;
								}
								else
								{
									(*createClass)->IncludeDerived = 0;
								}
							}

							att = att->Next;
						}
						/* grab the createClass value */

						innerXmlLen = ILibReadInnerXML(node, &innerXml);
						innerXml[innerXmlLen] = '\0';

						/* initialize to bad class */
						(*createClass)->MediaClass = CDS_CLASS_MASK_BADCLASS;
									
						/* determine object type */
						_DidlToCds_Helper_CopyUntilClassFragmentTerminator(classFragment, innerXml, MIN(innerXmlLen, CDS_MAX_CLASS_FRAGMENT_LEN), 1);
						indexIntoArray = _DidlToCds_Helper_FindStringInArray(classFragment, CDS_CLASS_OBJECT_TYPE, CDS_CLASS_OBJECT_TYPE_LEN);

						if (indexIntoArray > 0)
						{
							innerXml += ((int) strlen(CDS_CLASS_OBJECT_TYPE[indexIntoArray]) + 1);
							(*createClass)->MediaClass |= (indexIntoArray << CDS_SHIFT_OBJECT_TYPE);
										
							/* Determine major type */
							_DidlToCds_Helper_CopyUntilClassFragmentTerminator(classFragment, innerXml, MIN(innerXmlLen, CDS_MAX_CLASS_FRAGMENT_LEN), 0);
							indexIntoArray = _DidlToCds_Helper_FindStringInArray(classFragment, CDS_CLASS_MAJOR_TYPE, CDS_CLASS_MAJOR_TYPE_LEN);
							if (indexIntoArray > 0)
							{
								innerXml += ((int) strlen(CDS_CLASS_MAJOR_TYPE[indexIntoArray]) + 1);
								(*createClass)->MediaClass |= (indexIntoArray << CDS_SHIFT_MAJOR_TYPE);

								/* Determine minor type */
								_DidlToCds_Helper_CopyUntilClassFragmentTerminator(classFragment, innerXml, MIN(innerXmlLen, CDS_MAX_CLASS_FRAGMENT_LEN), 0);
								indexIntoArray = _DidlToCds_Helper_FindStringInArray(classFragment, CDS_CLASS_MAJOR_TYPE, CDS_CLASS_MAJOR_TYPE_LEN);
								if (indexIntoArray > 0)
								{
									(*createClass)->MediaClass |= (indexIntoArray << CDS_SHIFT_MINOR1_TYPE);
									/* TODO : Add vendor-specific supported minor types parsing here */
								}
							}
						}
					}
				}
			}
			else if (strcmp(node->Name, CDS_TAG_SEARCHCLASS) == 0)
			{
				prefixNS = ILibXML_LookupNamespace(node,node->NSTag,node->NSLength);
				if(strncmp(prefixNS, CDS_XML_NAMESPACE_UPNP, CDS_XML_NAMESPACE_UPNP_LEN)==0)
				{
					if((newObj->MediaClass & CDS_CLASS_MASK_OBJECT_TYPE) == CDS_CLASS_MASK_CONTAINER)
					{

						searchClass = &newObj->TypeObject.Container.SearchClass;					

						while((*searchClass)!=NULL)
						{
							searchClass = &(*searchClass)->Next;
						}

						(*searchClass) = (struct CdsSearchClass*) malloc(sizeof(struct CdsSearchClass));
						memset((*searchClass), 0, sizeof(struct CdsSearchClass));

						while (att != NULL)
						{
							att->Name[att->NameLength] = '\0';
							att->Value[att->ValueLength] = '\0';

							if (strcmp(att->Name, CDS_ATTRIB_INCLUDEDERIVED) == 0)
							{
								if (_DidlToCds_Helper_FindStringInArray(att->Value, CDS_TRUE_STRINGS, CDS_TRUE_STRINGS_LEN) >= 0)
								{
									/* set the includeDerived flag. */
									(*searchClass)->IncludeDerived = 1;
								}
								else
								{
									(*searchClass)->IncludeDerived = 0;
								}
							}

							att = att->Next;
						}
						/* grab the createClass value */

						innerXmlLen = ILibReadInnerXML(node, &innerXml);
						innerXml[innerXmlLen] = '\0';

						/* initialize to bad class */
						(*searchClass)->MediaClass = CDS_CLASS_MASK_BADCLASS;
									
						/* determine object type */
						_DidlToCds_Helper_CopyUntilClassFragmentTerminator(classFragment, innerXml, MIN(innerXmlLen, CDS_MAX_CLASS_FRAGMENT_LEN), 1);
						indexIntoArray = _DidlToCds_Helper_FindStringInArray(classFragment, CDS_CLASS_OBJECT_TYPE, CDS_CLASS_OBJECT_TYPE_LEN);

						if (indexIntoArray > 0)
						{
							innerXml += ((int) strlen(CDS_CLASS_OBJECT_TYPE[indexIntoArray]) + 1);
							(*searchClass)->MediaClass |= (indexIntoArray << CDS_SHIFT_OBJECT_TYPE);
										
							/* Determine major type */
							_DidlToCds_Helper_CopyUntilClassFragmentTerminator(classFragment, innerXml, MIN(innerXmlLen, CDS_MAX_CLASS_FRAGMENT_LEN), 0);
							indexIntoArray = _DidlToCds_Helper_FindStringInArray(classFragment, CDS_CLASS_MAJOR_TYPE, CDS_CLASS_MAJOR_TYPE_LEN);
							if (indexIntoArray > 0)
							{
								innerXml += ((int) strlen(CDS_CLASS_MAJOR_TYPE[indexIntoArray]) + 1);
								(*searchClass)->MediaClass |= (indexIntoArray << CDS_SHIFT_MAJOR_TYPE);

								/* Determine minor type */
								_DidlToCds_Helper_CopyUntilClassFragmentTerminator(classFragment, innerXml, MIN(innerXmlLen, CDS_MAX_CLASS_FRAGMENT_LEN), 0);
								indexIntoArray = _DidlToCds_Helper_FindStringInArray(classFragment, CDS_CLASS_MAJOR_TYPE, CDS_CLASS_MAJOR_TYPE_LEN);
								if (indexIntoArray > 0)
								{
									(*searchClass)->MediaClass |= (indexIntoArray << CDS_SHIFT_MINOR1_TYPE);
									/* TODO : Add vendor-specific supported minor types parsing here */
								}
							}
						}
					}
				}
			}
			else if (strcmp(node->Name, CDS_TAG_GENRE) == 0)
			{
				prefixNS = ILibXML_LookupNamespace(node,node->NSTag,node->NSLength);
				if(strncmp(prefixNS, CDS_XML_NAMESPACE_UPNP, CDS_XML_NAMESPACE_UPNP_LEN)==0)
				{
					innerXmlLen = ILibReadInnerXML(node, &innerXml);
					innerXml[innerXmlLen] = '\0';

					/* we need to unescape media data */
					innerXmlLen = ILibInPlaceXmlUnEscape(innerXml);

					newObj->DeallocateThese |= CDS_ALLOC_Genre;
					switch (newObj->MediaClass & CDS_CLASS_MASK_MAJOR)
					{

						/*
						* warning TODO: need to add support for multiple genres
						* this code only assumes that there's 1 <upnp:genre> tag
						*/

						case CDS_CLASS_MASK_MAJOR_AUDIOITEM:
							_DidlToCds_Helper_AddToArray(&newObj->TypeMajor.AudioItem.Genres, newObj->TypeMajor.AudioItem.NumGenres, innerXml, innerXmlLen);
							newObj->TypeMajor.AudioItem.NumGenres++;
							break;
						case CDS_CLASS_MASK_MAJOR_VIDEOITEM:
							_DidlToCds_Helper_AddToArray(&newObj->TypeMajor.VideoItem.Genres, newObj->TypeMajor.VideoItem.NumGenres, innerXml, innerXmlLen);
							newObj->TypeMajor.VideoItem.NumGenres++;
							break;
					}
					switch(newObj->MediaClass & CDS_CLASS_MASK_MINOR1)
					{
						case CDS_CLASS_MASK_MINOR1_MUSICALBUM:
							_DidlToCds_Helper_AddToArray(&newObj->TypeMinor1.MusicAlbum.Genres, newObj->TypeMinor1.MusicAlbum.NumGenres, innerXml, innerXmlLen);
							newObj->TypeMinor1.MusicAlbum.NumGenres++;
							break;
					}
					/*
						TODO: If you add more structs to union MajorTypes, MinorTypes1, MinorTypes2, 
						then add appropriate code here.
					*/
				}
			}
			else if (strcmp(node->Name, CDS_TAG_ALBUM) == 0)
			{
				prefixNS = ILibXML_LookupNamespace(node,node->NSTag,node->NSLength);
				if(strncmp(prefixNS, CDS_XML_NAMESPACE_UPNP, CDS_XML_NAMESPACE_UPNP_LEN)==0)
				{
					innerXmlLen = ILibReadInnerXML(node, &innerXml);
					innerXml[innerXmlLen] = '\0';

					/* we need to unescape media data */
					innerXmlLen = ILibInPlaceXmlUnEscape(innerXml);
					
					newObj->DeallocateThese |= CDS_ALLOC_Album;

					switch (newObj->MediaClass & CDS_CLASS_MASK_MAJOR)
					{
						case CDS_CLASS_MASK_MAJOR_AUDIOITEM:						
							_DidlToCds_Helper_AddToArray(&newObj->TypeMajor.AudioItem.Albums, newObj->TypeMajor.AudioItem.NumAlbums, innerXml, innerXmlLen);
							newObj->TypeMajor.AudioItem.NumAlbums++;
							break;
					}
					switch(newObj->MediaClass & CDS_CLASS_MASK_MINOR1)
					{
						case CDS_CLASS_MASK_MINOR1_PHOTO:
							_DidlToCds_Helper_AddToArray(&newObj->TypeMinor1.Photo.Albums, newObj->TypeMinor1.Photo.NumAlbums, innerXml, innerXmlLen);
							newObj->TypeMinor1.Photo.NumAlbums++;
							break;
					}
					/*
						TODO: If you add more structs to union MajorTypes, MinorTypes1, MinorTypes2, 
						then add appropriate code here.
					*/
				}
			}
			else if (strcmp(node->Name, CDS_TAG_CHANNELNAME) == 0)
			{
				prefixNS = ILibXML_LookupNamespace(node,node->NSTag,node->NSLength);
				if(strncmp(prefixNS, CDS_XML_NAMESPACE_UPNP, CDS_XML_NAMESPACE_UPNP_LEN)==0)
				{
					innerXmlLen = ILibReadInnerXML(node, &innerXml);
					innerXml[innerXmlLen] = '\0';

					/* we need to unescape media data */
					innerXmlLen = ILibInPlaceXmlUnEscape(innerXml);

					newObj->DeallocateThese |= CDS_ALLOC_ChannelName;

					switch(newObj->MediaClass & CDS_CLASS_MASK_MINOR1)
					{
						case CDS_CLASS_MASK_MINOR1_AUDIOBROADCAST:
							newObj->TypeMinor1.AudioBroadcast.ChannelName = (char*) malloc(innerXmlLen + 1);
							strcpy(newObj->TypeMinor1.AudioBroadcast.ChannelName, innerXml);
							break;
						case CDS_CLASS_MASK_MINOR1_VIDEOBROADCAST:
							newObj->TypeMinor1.VideoBroadcast.ChannelName = (char*) malloc(innerXmlLen + 1);
							strcpy(newObj->TypeMinor1.VideoBroadcast.ChannelName, innerXml);
							break;
					}
					/*
						TODO: If you add more structs to union MajorTypes, MinorTypes1, MinorTypes2, 
						then add appropriate code here.
					*/
				}
			}
			else if (strcmp(node->Name, CDS_TAG_CHANNELNR) == 0)
			{
				prefixNS = ILibXML_LookupNamespace(node,node->NSTag,node->NSLength);
				if(strncmp(prefixNS, CDS_XML_NAMESPACE_UPNP, CDS_XML_NAMESPACE_UPNP_LEN)==0)
				{
					innerXmlLen = ILibReadInnerXML(node, &innerXml);
					innerXml[innerXmlLen] = '\0';
					switch(newObj->MediaClass & CDS_CLASS_MASK_MINOR1)
					{
						case CDS_CLASS_MASK_MINOR1_AUDIOBROADCAST:
							ILibGetLong(innerXml, innerXmlLen, (long *)&(newObj->TypeMinor1.AudioBroadcast.ChannelNr));
							break;
						case CDS_CLASS_MASK_MINOR1_VIDEOBROADCAST:
							ILibGetLong(innerXml, innerXmlLen, (long *)&(newObj->TypeMinor1.VideoBroadcast.ChannelNr));
							break;
					}
					/*
						TODO: If you add more structs to union MajorTypes, MinorTypes1, MinorTypes2, 
						then add appropriate code here.
					*/
				}
			}
			else if (strcmp(node->Name, CDS_TAG_CONTAINERTYPE) == 0)
			{
				prefixNS = ILibXML_LookupNamespace(node,node->NSTag,node->NSLength);
				if(strncmp(prefixNS, CDS_XML_NAMESPACE_DLNA, CDS_XML_NAMESPACE_DLNA_LEN)==0)
				{
					innerXmlLen = ILibReadInnerXML(node, &innerXml);
					innerXml[innerXmlLen] = '\0';
					newObj->TypeObject.Container.DlnaContainerType = 1;
				}
			}
			else if (strcmp(node->Name, CDS_TAG_TAKEOUTGROUP) == 0)
			{
				prefixNS = ILibXML_LookupNamespace(node,node->NSTag,node->NSLength);
				if(strncmp(prefixNS, CDS_XML_NAMESPACE_DLNA, CDS_XML_NAMESPACE_DLNA_LEN)==0)
				{
					/* TODO: Add support for take out */
					//innerXmlLen = ILibReadInnerXML(node, &innerXml);
					//innerXml[innerXmlLen] = '\0';


					//newObj->TakeOutGroups = (char**) malloc(1 * sizeof(char*));
					//memset(newObj->TakeOutGroups, 0, 1 * sizeof(char*));
					//newObj->TakeOutGroups[0] = (char*) malloc(innerXmlLen + 1);
					//strcpy(newObj->TakeOutGroups[0], innerXml);
					//newObj->NumTakeOutGroups = 1;
					//newObj->DeallocateThese |= CDS_ALLOC_TakeOutGroups;
				}
			}

			/* free attribute mapping */
			ILibDestructXMLAttributeList(attribs);
		}

		node = node->Next;
		#ifdef _DEBUG
		if (node == NULL)
		{
			printf("CDS_DeserializeDidlToObject: Unexpected null node.\r\n");
			ASSERT(0);
		}
		#endif

	}


	/* amount of data needed to store various pointer-based fields */
	dataSize = _DidlToCds_Helper_GetRequiredSizeForMediaObject(newObj);

	// ToDo: additional AllocateCdsObject() method needed

	/* total size needed for the string data and the CDS media object struct */
	mallocSize = dataSize + sizeof(struct CdsObject) + 1;

	/* initialize fields on CDS object */
	newObj->CpInfo.Reserved.ReservedRefCount = 0;
	newObj->CpInfo.Reserved.ReservedMallocSize = mallocSize;

	return newObj;
}

void _CDS_Clone_MultipleStrings(char ***strings, int num_strings, const char** clone_this)
{
	int i;

	if ((num_strings > 0) && (clone_this != NULL))
	{
		(*strings) = (char**) malloc(num_strings * sizeof(char*));
		memset((*strings), 0, num_strings * sizeof(char*));

		for (i=0; i < num_strings; i++)
		{
			/* deallocate each album string */
			if (clone_this[i] != NULL)
			{
				(*strings)[i] = (char*) malloc(strlen(clone_this[i]) + 1);
				strcpy((*strings)[i], clone_this[i]);
			}
		}
	}
}

struct CdsObject* CDS_CloneMediaObject(const struct CdsObject *cloneThis)
{
	struct CdsObject *newObj = NULL;
	const struct CdsObject *ct;
	struct CdsResource *ctr, **newRes;
	struct CdsCreateClass *ctcc, **newCreateClass;
	struct CdsSearchClass *ctsc, **newSearchClass;
	ct = cloneThis;

	if (ct != NULL)
	{
			
		/* allocate memory for the cloned CDS object and copy its state*/
		newObj = CDS_AllocateObject();

		newObj->MediaClass = ct->MediaClass;
		newObj->DlnaManaged = ct->DlnaManaged;
		newObj->Flags = ct->Flags;
		newObj->NumTakeOutGroups = ct->NumTakeOutGroups;
		newObj->User = ct->User;
		/* we will clone resources in another step */
		newObj->Res = NULL;

		if (ct->CpInfo.Reserved.ServiceObject != NULL)
		{
			#ifdef MSCP
			MSCP_AddRefRootDevice(ct->CpInfo.Reserved.ServiceObject);
			#endif
			newObj->CpInfo.Reserved.ServiceObject = ct->CpInfo.Reserved.ServiceObject;
		}
		if(ct->Source != NULL)
		{
			newObj->Source = (char*) malloc(strlen(ct->Source) + 1);
			strcpy(newObj->Source, ct->Source);
			newObj->DeallocateThese |= CDS_ALLOC_ID;
		}
		if(ct->ID != NULL)
		{
			newObj->ID = (char*) malloc(strlen(ct->ID) + 1);
			strcpy(newObj->ID, ct->ID);
			newObj->DeallocateThese |= CDS_ALLOC_ID;
		}
		if(ct->ParentID != NULL)
		{
			newObj->ParentID = (char*) malloc(strlen(ct->ParentID) + 1);
			strcpy(newObj->ParentID, ct->ParentID);
			newObj->DeallocateThese |= CDS_ALLOC_ParentID;
		}
		if(ct->Title != NULL)
		{
			newObj->Title = (char*) malloc(strlen(ct->Title) + 1);
			strcpy(newObj->Title, ct->Title);
			newObj->DeallocateThese |= CDS_ALLOC_Title;
		}
		if(ct->Creator != NULL)
		{
			newObj->Creator = (char*) malloc(strlen(ct->Creator) + 1);
			strcpy(newObj->Creator, ct->Creator);
			newObj->DeallocateThese |= CDS_ALLOC_Creator;
		}

		switch (ct->MediaClass & CDS_CLASS_MASK_OBJECT_TYPE)
		{
			case CDS_CLASS_MASK_CONTAINER:
				newObj->TypeObject.Container.ChildCount = ct->TypeObject.Container.ChildCount;
				newObj->TypeObject.Container.DlnaContainerType = ct->TypeObject.Container.DlnaContainerType;
				
				/*
				*	Clone CreateClass.
				*/
				ctcc = ct->TypeObject.Container.CreateClass;
				newCreateClass = &(newObj->TypeObject.Container.CreateClass);

				while (ctcc != NULL)
				{
					/* allocate and copy */
					(*newCreateClass) = (struct CdsCreateClass*) malloc(sizeof (struct CdsCreateClass));
					memset((*newCreateClass), 0, sizeof (struct CdsCreateClass));
					(*newCreateClass)->IncludeDerived = ctcc->IncludeDerived;
					(*newCreateClass)->MediaClass = ctcc->MediaClass;
					newCreateClass = &((*newCreateClass)->Next);
					ctcc = ctcc->Next;
				}

				/*
				*	Clone SearchClass.
				*/
				ctsc = ct->TypeObject.Container.SearchClass;
				newSearchClass = &(newObj->TypeObject.Container.SearchClass);

				while (ctsc != NULL)
				{
					/* allocate and copy */
					(*newSearchClass) = (struct CdsSearchClass*) malloc(sizeof (struct CdsSearchClass));
					memset((*newSearchClass), 0, sizeof (struct CdsSearchClass));
					(*newSearchClass)->IncludeDerived = ctsc->IncludeDerived;
					(*newSearchClass)->MediaClass = ctsc->MediaClass;
					newSearchClass = &((*newSearchClass)->Next);
					ctsc = ctsc->Next;
				}
				break;
			case CDS_CLASS_MASK_ITEM:
				if(ct->TypeObject.Item.RefID != NULL)
				{
					newObj->TypeObject.Item.RefID = (char*) malloc(strlen(ct->TypeObject.Item.RefID) + 1);
					strcpy(newObj->TypeObject.Item.RefID, ct->TypeObject.Item.RefID);
					newObj->DeallocateThese |= CDS_ALLOC_RefID;
				}
				break;
		}

		switch (ct->MediaClass & CDS_CLASS_MASK_MAJOR)
		{
			case CDS_CLASS_MASK_MAJOR_AUDIOITEM:
				if(ct->TypeMajor.AudioItem.Albums != NULL && ct->TypeMajor.AudioItem.NumAlbums > 0)
				{
					newObj->TypeMajor.AudioItem.NumAlbums = ct->TypeMajor.AudioItem.NumAlbums;
					_CDS_Clone_MultipleStrings(&newObj->TypeMajor.AudioItem.Albums, ct->TypeMajor.AudioItem.NumAlbums, (const char**)ct->TypeMajor.AudioItem.Albums);
					newObj->DeallocateThese |= CDS_ALLOC_Album;
				}
				if(ct->TypeMajor.AudioItem.Genres != NULL && ct->TypeMajor.AudioItem.NumGenres > 0)
				{
					newObj->TypeMajor.AudioItem.NumGenres = ct->TypeMajor.AudioItem.NumGenres;
					_CDS_Clone_MultipleStrings(&newObj->TypeMajor.AudioItem.Genres, ct->TypeMajor.AudioItem.NumGenres, (const char**)ct->TypeMajor.AudioItem.Genres);
					newObj->DeallocateThese |= CDS_ALLOC_Genre;
				}
				newObj->TypeMajor.AudioItem.Date = ct->TypeMajor.AudioItem.Date;	
				break;
			case CDS_CLASS_MASK_MAJOR_IMAGEITEM:
				newObj->TypeMajor.ImageItem.Date = ct->TypeMajor.ImageItem.Date;			
				break;
			case CDS_CLASS_MASK_MAJOR_VIDEOITEM:
				if(ct->TypeMajor.VideoItem.Genres != NULL && ct->TypeMajor.VideoItem.NumGenres > 0)
				{
					newObj->TypeMajor.VideoItem.NumGenres = ct->TypeMajor.VideoItem.NumGenres;
					_CDS_Clone_MultipleStrings(&newObj->TypeMajor.VideoItem.Genres, ct->TypeMajor.VideoItem.NumGenres, (const char**)ct->TypeMajor.VideoItem.Genres);
					newObj->DeallocateThese |= CDS_ALLOC_Genre;
				}
				newObj->TypeMajor.VideoItem.Date = ct->TypeMajor.VideoItem.Date;					
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

		switch (ct->MediaClass & CDS_CLASS_MASK_MINOR1)
		{
			case CDS_CLASS_MASK_MINOR1_MUSICALBUM:
				if(ct->TypeMinor1.MusicAlbum.Genres != NULL && ct->TypeMinor1.MusicAlbum.NumGenres > 0)
				{
					newObj->TypeMinor1.MusicAlbum.NumGenres = ct->TypeMinor1.MusicAlbum.NumGenres;
					_CDS_Clone_MultipleStrings(&newObj->TypeMinor1.MusicAlbum.Genres, ct->TypeMinor1.MusicAlbum.NumGenres, (const char**)ct->TypeMinor1.MusicAlbum.Genres);				
					newObj->DeallocateThese |= CDS_ALLOC_Genre;
				}			
				break;
			case CDS_CLASS_MASK_MINOR1_PHOTO:
				if(ct->TypeMinor1.Photo.Albums != NULL && ct->TypeMinor1.Photo.NumAlbums > 0)
				{
					newObj->TypeMinor1.Photo.NumAlbums = ct->TypeMinor1.Photo.NumAlbums;
					_CDS_Clone_MultipleStrings(&newObj->TypeMinor1.Photo.Albums, ct->TypeMinor1.Photo.NumAlbums, (const char**)ct->TypeMinor1.Photo.Albums);
					newObj->DeallocateThese |= CDS_ALLOC_Album;
				}		
				break;
			case CDS_CLASS_MASK_MINOR1_AUDIOBROADCAST:
				if(ct->TypeMinor1.AudioBroadcast.ChannelName != NULL)
				{
					newObj->TypeMinor1.AudioBroadcast.ChannelName = (char*) malloc(strlen(ct->TypeMinor1.AudioBroadcast.ChannelName) + 1);
					strcpy(newObj->TypeMinor1.AudioBroadcast.ChannelName, ct->TypeMinor1.AudioBroadcast.ChannelName);
					newObj->DeallocateThese |= CDS_ALLOC_ChannelName;
				}
				newObj->TypeMinor1.AudioBroadcast.ChannelNr = ct->TypeMinor1.AudioBroadcast.ChannelNr;
				break;
			case CDS_CLASS_MASK_MINOR1_VIDEOBROADCAST:
				if(ct->TypeMinor1.VideoBroadcast.ChannelName != NULL)
				{
					newObj->TypeMinor1.VideoBroadcast.ChannelName = (char*) malloc(strlen(ct->TypeMinor1.VideoBroadcast.ChannelName) + 1);
					strcpy(newObj->TypeMinor1.VideoBroadcast.ChannelName, ct->TypeMinor1.VideoBroadcast.ChannelName);
					newObj->DeallocateThese |= CDS_ALLOC_ChannelName;
				}
				newObj->TypeMinor1.VideoBroadcast.ChannelNr = ct->TypeMinor1.VideoBroadcast.ChannelNr;
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

		/*
		*	Clone resources.
		*/
		ctr = ct->Res;
		newRes = &(newObj->Res);

		while (ctr != NULL)
		{
			/* allocate and copy */
			(*newRes) = CDS_AllocateResource();

			(*newRes)->Bitrate = ctr->Bitrate;
			(*newRes)->BitsPerSample = ctr->BitsPerSample;
			(*newRes)->ColorDepth = ctr->ColorDepth;
			(*newRes)->Duration = ctr->Duration;
			(*newRes)->Flags = ctr->Flags;
			(*newRes)->NrAudioChannels = ctr->NrAudioChannels;
			(*newRes)->ResolutionX = ctr->ResolutionX;
			(*newRes)->ResolutionY = ctr->ResolutionY;
			(*newRes)->ResumeUpload = ctr->ResumeUpload;
			(*newRes)->SampleFrequency = ctr->SampleFrequency;
			(*newRes)->Size = ctr->Size;
			(*newRes)->TrackTotal = ctr->TrackTotal;
			(*newRes)->UploadedSize = ctr->UploadedSize;

			if(ctr->Value != NULL)
			{
				(*newRes)->Value = (char*) malloc(strlen(ctr->Value) + 1);
				strcpy((*newRes)->Value, ctr->Value);
				(*newRes)->Allocated |= CDS_RES_ALLOC_Value;
			}
			if(ctr->ProtocolInfo != NULL)
			{
				(*newRes)->ProtocolInfo = (char*) malloc(strlen(ctr->ProtocolInfo) + 1);
				strcpy((*newRes)->ProtocolInfo, ctr->ProtocolInfo);
				(*newRes)->Allocated |= CDS_RES_ALLOC_ProtocolInfo;
			}
			if(ctr->Protection != NULL)
			{
				(*newRes)->Protection = (char*) malloc(strlen(ctr->Protection) + 1);
				strcpy((*newRes)->Protection, ctr->Protection);
				(*newRes)->Allocated |= CDS_RES_ALLOC_Protection;
			}
			if(ctr->ImportUri != NULL)
			{
				(*newRes)->ImportUri = (char*) malloc(strlen(ctr->ImportUri) + 1);
				strcpy((*newRes)->ImportUri, ctr->ImportUri);
				(*newRes)->Allocated |= CDS_RES_ALLOC_ImportUri;
			}
			if(ctr->IfoFileUri != NULL)
			{
				(*newRes)->IfoFileUri = (char*) malloc(strlen(ctr->IfoFileUri) + 1);
				strcpy((*newRes)->IfoFileUri, ctr->IfoFileUri);
				(*newRes)->Allocated |= CDS_RES_ALLOC_IfoFileUri;
			}
			if(ctr->ImportIfoFileUri != NULL)
			{
				(*newRes)->ImportIfoFileUri = (char*) malloc(strlen(ctr->ImportIfoFileUri) + 1);
				strcpy((*newRes)->ImportIfoFileUri, ctr->ImportIfoFileUri);
				(*newRes)->Allocated |= CDS_RES_ALLOC_ImportIfoFileUri;
			}

			newRes = &((*newRes)->Next);
			ctr = ctr->Next;
		}
	}

	return newObj;
}

void CDS_ObjRef_Add(struct CdsObject *refThis)
{

	ASSERT(refThis != NULL);

	sem_wait(&(refThis->CpInfo.Reserved.ReservedLock));
	refThis->CpInfo.Reserved.ReservedRefCount++;
	sem_post(&(refThis->CpInfo.Reserved.ReservedLock));
}

void CDS_ObjRef_Release(struct CdsObject *releaseThis)
{

	/*
	 *	Remember that the ResultsList is stored in such a way that
	 *	each MediaObject and MediaResource pointer is allocated
	 *	with additional trailinng memory that stores the strings associated
	 *	with the object. Therefore, when we free a MediaResource or
	 *	a MediaObject, we're also freeing the memory for the string metadata.
	 */

	if (releaseThis != NULL)
	{
		sem_wait(&(releaseThis->CpInfo.Reserved.ReservedLock));
		releaseThis->CpInfo.Reserved.ReservedRefCount--;

		if (releaseThis->CpInfo.Reserved.ReservedRefCount < 0)
		{
			if (releaseThis->CpInfo.Reserved.ServiceObject != NULL)
			{
				#ifdef MSCP
				/* release the UPnP device */
				MSCP_ReleaseRootDevice(releaseThis->CpInfo.Reserved.ServiceObject);
				releaseThis->CpInfo.Reserved.ServiceObject = NULL;		

				#endif
			}

			sem_post(&(releaseThis->CpInfo.Reserved.ReservedLock));

			/* destroy the object */
			CDS_DestroyObject(releaseThis);
		}
		else
		{
			sem_post(&(releaseThis->CpInfo.Reserved.ReservedLock));
		}
	}
}
