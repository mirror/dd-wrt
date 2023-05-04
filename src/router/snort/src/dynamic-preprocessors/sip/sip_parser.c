/****************************************************************************
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2011-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************
 * Provides convenience functions for parsing and querying configuration.
 *
 * 2/17/2011 - Initial implementation ... Hui Cao <hcao@sourcefire.com>
 *
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef HAVE_PARSER_H
#include <ctype.h>
#include "sf_types.h"
#include "sf_snort_packet.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"
#include "sip_parser.h"
#include "spp_sip.h"
#include "sip_config.h"
#include "sip_utils.h"
#include "sf_ip.h"

#ifdef DUMP_BUFFER
#include "sip_buffer_dump.h"
#endif

#define MAX_NUM_32BIT  2147483647


#define SIP_PARSE_NOFOLDING	 (-2)
#define SIP_PARSE_ERROR		 (-1)
#define SIP_PARSE_SUCCESS	 (1)

/*Should at least have SIP/2.0 */
#define SIP_KEYWORD          "SIP/"
#define SIP_KEYWORD_LEN      4
#define SIP_VERSION_NUM_LEN  3  /*2.0 or 1.0 or 1.1*/
#define SIP_VERSION_LEN      SIP_KEYWORD_LEN + SIP_VERSION_NUM_LEN
#define SIP_MIN_MSG_LEN      SIP_VERSION_LEN

#define SIP_TAG_KEYWORD		 "tag="
#define SIP_TAG_KEYWORD_LEN      4

static int sip_headers_parse(SIPMsg *, const char *, char *,char **);
static int sip_startline_parse(SIPMsg *, const char *, char *,char **);
static int sip_body_parse(SIPMsg *, const char *, char *, char **);
static int sip_check_headers(SIPMsg *);

static int sip_parse_via(SIPMsg *, const char *, const char *);
static int sip_parse_from(SIPMsg *, const char *, const char *);
static int sip_parse_to(SIPMsg *, const char *, const char *);
static int sip_parse_call_id(SIPMsg *, const char *, const char *);
static int sip_parse_user_agent(SIPMsg *, const char *, const char *);
static int sip_parse_server(SIPMsg *, const char *, const char *);
static int sip_parse_cseq(SIPMsg *, const char *, const char *);
static int sip_parse_contact(SIPMsg *, const char *, const char *);
static int sip_parse_authorization(SIPMsg *, const char *, const char *);
static int sip_parse_content_type(SIPMsg *, const char *, const char *);
static int sip_parse_content_len(SIPMsg *, const char *, const char *);
static int sip_parse_content_encode(SIPMsg *, const char *, const char *);
static int sip_process_headField(SIPMsg *, const char *, const char *, int *);
static int sip_process_bodyField(SIPMsg *, const char *, const char *);
static int sip_parse_sdp_o(SIPMsg *, const char *, const char *);
static int sip_parse_sdp_c(SIPMsg *, const char *, const char *);
static int sip_parse_sdp_m(SIPMsg *, const char *, const char *);
static int sip_find_linebreak(const char *, char *, char **);

/*
 * Header fields and processing functions
 */
typedef struct _SIPheaderField
{
	char *fname;
	int fnameLen;
	char *shortName;
	int (*setfield) (SIPMsg *, const char *,const char *);

} SIPheaderField;

/*
 * Body fields and processing functions
 */
typedef struct _SIPbodyField
{
	char *fname;
	int fnameLen;
	int (*setfield) (SIPMsg *, const char *,const char *);

} SIPbodyField;

/*
 * header field name, short form field name, and field processing function
 */

SIPheaderField headerFields[] =
{
		{"Via", 3, "v",  &sip_parse_via},
		{"From", 4,"f",  &sip_parse_from},
		{"To", 2, "t",  &sip_parse_to},
		{"Call-ID", 7, "i", &sip_parse_call_id},
		{"CSeq", 4, NULL, &sip_parse_cseq},
		{"Contact", 7, "m", &sip_parse_contact},
		{"Authorization", 13, NULL,  &sip_parse_authorization},
		{"Content-Type", 12, "c",  &sip_parse_content_type},
		{"Content-Length", 14, "l",  &sip_parse_content_len},
		{"Content-Encoding", 16, "e", &sip_parse_content_encode},
		{"User-Agent", 10, NULL, &sip_parse_user_agent},
		{"Server", 6, NULL, &sip_parse_server},
		{NULL, 0, NULL, NULL}
};

/*
 * body field name, field processing function
 */

SIPbodyField bodyFields[] =
{
		{"o=", 2, &sip_parse_sdp_o},
		{"c=", 2, &sip_parse_sdp_c},
		{"m=", 2, &sip_parse_sdp_m},
		{NULL, 0, NULL}
};

/********************************************************************
 * Function: sip_process_headField()
 *
 * Process the header fields (lines). This also deals with folding.
 *
 * Arguments:
 *  SIPMsg *    - sip message
 *  char* start - start of the header line
 *  char* end   - end of the header line
 *  int*        - index of last field processed. Used for folding processing
 *                This value will be updated after current field been processed
 * Returns:
 *  SIP_PARSE_ERROR
 *  SIP_PARSE_SUCCESS
 ********************************************************************/
static int sip_process_headField(SIPMsg *msg, const char *start, const char *end, int *lastFieldIndex)
{
	int findex =0;
	int length = end -start;
	char *colonIndex;
    char *newStart, *newEnd, newLength;
	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "process line: %.*s\n", length, start));

	// If this is folding
	if((' ' == start[0]) || ('\t' == start[0]))
	{
		if(SIP_PARSE_NOFOLDING != *lastFieldIndex)
		{
			SIP_TrimSP(start, end, &newStart, &newEnd);
			return(headerFields[*lastFieldIndex].setfield(msg, newStart, newEnd));
		}
	}
	// Otherwise, continue normal processing
	colonIndex = memchr(start, ':', length);

	if (!colonIndex || (colonIndex < start + 1))
		return SIP_PARSE_ERROR;

	if (!SIP_TrimSP(start, colonIndex, &newStart, &newEnd))
		return SIP_PARSE_ERROR;

	newLength =  newEnd - newStart;

	/*Find out whether the field name needs to process*/
	while (NULL != headerFields[findex].fname)
	{
		//Use the full name to check
		if ((headerFields[findex].fnameLen == newLength)&&
				(0 == strncasecmp(headerFields[findex].fname, newStart, newLength)))
		{
			break;
		}
		//Use short name to check
		else if ((NULL != headerFields[findex].shortName) &&
				( 1 == newLength)&&
				(0 == strncasecmp(headerFields[findex].shortName, newStart, newLength)))
		{
			break;
		}
		findex++;
	}

	if (NULL != headerFields[findex].fname)
	{
		// Found the field name, evaluate the value
		SIP_TrimSP(colonIndex + 1, end, &newStart, &newEnd);
		*lastFieldIndex = findex;
		return (headerFields[findex].setfield(msg, newStart, newEnd));
	}
	*lastFieldIndex = SIP_PARSE_NOFOLDING;
	return SIP_PARSE_SUCCESS;
}
/********************************************************************
 * Function: sip_process_bodyField()
 *
 * Process the body fields.
 *
 * Arguments:
 *  SIPMsg *    - sip message
 *  char* start - start of the line
 *  char* end   - end of the line
 *
 * Returns:
 *  SIP_PARSE_ERROR
 *  SIP_PARSE_SUCCESS
 ********************************************************************/
static int sip_process_bodyField(SIPMsg *msg, const char *start, const char *end)
{
	int findex =0;
	if (start == end)
		return SIP_PARSE_SUCCESS;
	/*Find out whether the field name needs to process*/
	while (NULL != bodyFields[findex].fname)
	{
		int length = bodyFields[findex].fnameLen;
		if (0 == strncasecmp(bodyFields[findex].fname, start,length))
		{
			return (bodyFields[findex].setfield(msg,start + length, end));
		}

		findex++;
	}
	return SIP_PARSE_SUCCESS;
}
/********************************************************************
 * Function: sip_find_linebreak()
 *
 * Find the line break \r \n in the current buffer
 *
 * Arguments:
 *  char* start - start of the buffer
 *  char* end   - end of the buffer
 *  char **lineEnd - output, point to the end of the line defined by line breaks
 * Returns:
 *  int - number of line breaks found in the line found.
 ********************************************************************/
static int sip_find_linebreak(const char *start, char *end, char **lineEnd)
{
	int numCRLF;
	char *s = (char *)start;
	*lineEnd = NULL;
    numCRLF = 0;
    if (start >= end)
        return 0;

    while ((s < end) && !('\r' ==*s || '\n' == *s))
	{
		s++;
	}

    if (s == end)
    	return 0;

    s++;
    numCRLF = 1;

    if ((s < end) && ('\r' == s[-1]) && ('\n' == s[0]))
	{
		s++;
		numCRLF = 2;
	}

	*lineEnd= s;
	return numCRLF;
}
/********************************************************************
 * Function: sip_is_valid_version()
 *
 * Check whether the version is a valid version (2.0, 1.1, 1.0)
 *
 * Arguments:
 *  char* start - start of the version
 *
 * Returns:
 *   SIP_TRUE
 *   SIP_FALSE
 ********************************************************************/
static inline int sip_is_valid_version(const char *start)
{
    if (!strncmp(start, "1.", 2))
    {
        if ((*(start+2) == '1') || (*(start+2) == '0'))
            return SIP_TRUE;
    }
    else if (!strncmp(start, "2.0", 3))
        return SIP_TRUE;

    return SIP_FALSE;
}
/********************************************************************
 * Function: sip_startline_parse()
 *
 * Parse the start line: request and response are different
 *
 * Arguments:
 *  SIPMsg *    - sip message
 *  char* buff  - start of the sip message buffer
 *  char* end   - end of the buffer
 *  char**lineEnd - output, the found end of start line
 * Returns:
 *  SIP_FAILURE
 *  SIP_SUCCESS
 ********************************************************************/

static int sip_startline_parse(SIPMsg *msg, const char *buff, char *end, char **lineEnd)
{
	char *next;
	char *start;
	int length;
	int numOfLineBreaks;

	start = (char *) buff;

	numOfLineBreaks = sip_find_linebreak(start, end, &next);
	if (numOfLineBreaks < 1)
	{
		/*No CRLF */
		DEBUG_WRAP(DebugMessage(DEBUG_SIP, "No CRLF, check failed\n"));
		return SIP_FAILURE;
	}

	/*Exclude CRLF from start line*/
	length =  next - start - numOfLineBreaks;

	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Start line: %.*s \n", length, start));
	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "End of Start line \n"));

	/*Should at least have SIP/2.0 */
	if (length < SIP_MIN_MSG_LEN)
	{
		DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Message too short, check failed\n"));
			return SIP_FAILURE;
	}

	*lineEnd = next;
	// This is a response
	if (0 == strncmp((const char *) buff, (const char *) SIP_KEYWORD, SIP_KEYWORD_LEN))
	{
		char *space;
		unsigned long statusCode;

		/*Process response*/
		msg->method = NULL;
		msg->uri = NULL;

        /*Check SIP version number, end with SP*/
        if (!(sip_is_valid_version(buff + SIP_KEYWORD_LEN) && (*(buff + SIP_VERSION_LEN) == ' ')))
        {
            ALERT(SIP_EVENT_INVALID_VERSION,SIP_EVENT_INVALID_VERSION_STR);
        }

		space = strchr(buff, ' ');
		if (space == NULL)
			return SIP_FAILURE;
		statusCode = _dpd.SnortStrtoul(space + 1, NULL, 10);
		if (( statusCode > MAX_STAT_CODE) || (statusCode < MIN_STAT_CODE ))
		{
			ALERT(SIP_EVENT_BAD_STATUS_CODE,SIP_EVENT_BAD_STATUS_CODE_STR)
    		msg->status_code =  MAX_STAT_CODE + 1;
		}
        else
		    msg->status_code =  (uint16_t)statusCode;
		DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Status code: %d \n", msg->status_code));

        }
	else  /* This might be a request*/
	{
		char *space;
		char *version;
		int length;
		SIPMethodNode *method;

		/*Process request*/
		if (NULL ==sip_eval_config)
			return SIP_FAILURE;
		msg->status_code = 0;

        // Parse the method
		space = memchr(buff, ' ', end - buff);
		if (space == NULL)
			return SIP_FAILURE;
		length = space - buff;
		msg->method = (char*)buff;
		msg->methodLen = length;
		DEBUG_WRAP(DebugMessage(DEBUG_SIP, "method: %.*s\n", msg->methodLen, msg->method));

#ifdef DUMP_BUFFER
                dumpBuffer(METHOD_DUMP,msg->method,msg->methodLen);
#endif

		method = SIP_FindMethod (sip_eval_config->methods, msg->method, msg->methodLen);
		if (method)
		{
		    msg->methodFlag = method->methodFlag;
		    DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Found the method: %s, Flag: 0x%x\n", method->methodName, method->methodFlag));
		}

		// parse the uri
		if (space + 1 > end)
			return SIP_FAILURE;
		msg->uri = space + 1;
		space = memchr(space + 1, ' ', end - msg->uri);
		if (space == NULL)
			return SIP_FAILURE;
		msg->uriLen = space - msg->uri;

#ifdef DUMP_BUFFER
                dumpBuffer(URI_DUMP,msg->uri,msg->uriLen);
#endif

		DEBUG_WRAP(DebugMessage(DEBUG_SIP, "uri: %.*s, length: %u\n", msg->uriLen, msg->uri, msg->uriLen));
		if(0 == msg->uriLen)
			ALERT(SIP_EVENT_EMPTY_REQUEST_URI,SIP_EVENT_EMPTY_REQUEST_URI_STR)
		else if (sip_eval_config->maxUriLen && (msg->uriLen > sip_eval_config->maxUriLen))
			ALERT(SIP_EVENT_BAD_URI,SIP_EVENT_BAD_URI_STR);

		version = space + 1;
		if (version + SIP_VERSION_LEN > end)
		    return SIP_FAILURE;
		if (0 != strncmp((const char *) version, (const char *) SIP_KEYWORD, SIP_KEYWORD_LEN))
		    return SIP_FAILURE;
		/*Check SIP version number, end with CRLF*/
		if (!sip_is_valid_version(*lineEnd - SIP_VERSION_NUM_LEN - numOfLineBreaks))
		{
		    ALERT(SIP_EVENT_INVALID_VERSION,SIP_EVENT_INVALID_VERSION_STR);
		}

        if (NULL == method)
        {
            ALERT(SIP_EVENT_UNKOWN_METHOD, SIP_EVENT_UNKOWN_METHOD_STR);
            return SIP_FAILURE;
        }
	}

	return SIP_SUCCESS;
}
/********************************************************************
 * Function: sip_headers_parse()
 *
 * Parse the SIP header: request and response are the same
 *
 * Arguments:
 *  SIPMsg *    - sip message
 *  char* buff  - start of the header
 *  char* end   - end of the buffer
 *  char**lineEnd - output, the found end of header
 * Returns:
 *  SIP_FAILURE
 *  SIP_SUCCESS
 ********************************************************************/
static int sip_headers_parse(SIPMsg *msg, const char *buff, char *end, char **headEnd)
{
	char *next;
	char *start;
	int length;
	int numOfLineBreaks;
    int lastFieldIndex = SIP_PARSE_NOFOLDING ;

	start = (char *) buff;
	/*
	 * The end of header is defined by two CRLFs, or CRCR, or LFLF
	 */
	numOfLineBreaks = sip_find_linebreak(start, end, &next);

	while (numOfLineBreaks > 0)
	{

		/*Processing this line*/
		length =  next - start - numOfLineBreaks;

		DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Header line: %.*s\n", length, start));
		/*Process headers*/
		sip_process_headField(msg, start, start + length, &lastFieldIndex);

		/*check the end of header*/
		if ((1 == numOfLineBreaks) &&  ( start[0] == start[-1]))
		{
			/*Either CRCR or LFLF*/
			*headEnd = next ;
			return SIP_SUCCESS;

		}
		else if ( (2 == numOfLineBreaks) && ('\r' == start[0])&&('\n' == start[1]))
		{
			*headEnd = next;
			return SIP_SUCCESS;
		}

		start = next;
		numOfLineBreaks = sip_find_linebreak(start, end, &next);

	}
	return SIP_SUCCESS;
}
/********************************************************************
 * Function: sip_body_parse()
 *
 * Parse the SIP body: request and response are the same
 *
 * Arguments:
 *  SIPMsg *    - sip message
 *  char* buff  - start of the body
 *  char* end   - end of the buffer
 *  char**lineEnd - output, the found end of body
 * Returns:
 *  SIP_FAILURE
 *  SIP_SUCCESS
 ********************************************************************/
static int sip_body_parse(SIPMsg *msg, const char *buff, char *end, char **bodyEnd)
{
	int length;
	char *next;
	char *start;
	int numOfLineBreaks;
	length = end - buff;
	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Body length: %d\n", length););
	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Body line: %.*s\n", length, buff););

	// Initialize it
	*bodyEnd = end;

	if (buff == end)
    	return SIP_SUCCESS;

    msg->body_data = (uint8_t *)buff;

    // Create a media session
    msg->mediaSession = (SIP_MediaSession *)_dpd.snortAlloc(1,
                                                sizeof(SIP_MediaSession),
                                                PP_SIP, PP_MEM_CATEGORY_SESSION);
    if (NULL == msg->mediaSession)
    	return SIP_FAILURE;
    start = (char *) buff;

	/*
	 * The end of body is defined by two CRLFs or CRCR or LFLF
	 */
	numOfLineBreaks = sip_find_linebreak(start, end, &next);

	while (numOfLineBreaks > 0)
	{
		/*Processing this line*/
		length =  next - start - numOfLineBreaks;

		DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Body line: %.*s\n", length, start));
		/*Process body fields*/
		sip_process_bodyField(msg, start, start + length);

		start = next;
		numOfLineBreaks = sip_find_linebreak(start, end, &next);
	}
	*bodyEnd = start;
	return SIP_SUCCESS;
}

/********************************************************************
 * Function: sip_check_headers()
 *
 * Check whether the headers are mal-formed.
 * Most checks are here, except some need context information are scattered
 * in the parsing.
 *
 * Arguments:
 *  SIPMsg *    - sip message
 *
 * Returns:
 *  SIP_FAILURE
 *  SIP_SUCCESS
 ********************************************************************/
static int sip_check_headers(SIPMsg *msg)
{
	int ret = SIP_SUCCESS;
	if(0 == msg->fromLen)
	{
		ALERT(SIP_EVENT_EMPTY_FROM,SIP_EVENT_EMPTY_FROM_STR)
		ret =  SIP_FAILURE;
	}
	else if (sip_eval_config->maxFromLen && (msg->fromLen > sip_eval_config->maxFromLen))
	{
		ALERT(SIP_EVENT_BAD_FROM,SIP_EVENT_BAD_FROM_STR);
		ret = SIP_FAILURE;
	}

	if(0 == msg->toLen)
	{
		ALERT(SIP_EVENT_EMPTY_TO,SIP_EVENT_EMPTY_TO_STR)
		ret = SIP_FAILURE;
	}
	else if (sip_eval_config->maxToLen && (msg->toLen > sip_eval_config->maxToLen))
	{
		ALERT(SIP_EVENT_BAD_TO,SIP_EVENT_BAD_TO_STR);
		ret = SIP_FAILURE;
	}

	if(0 == msg->callIdLen)
	{
		ALERT(SIP_EVENT_EMPTY_CALL_ID,SIP_EVENT_EMPTY_CALL_ID_STR)
		ret = SIP_FAILURE;
	}
	else if ( sip_eval_config->maxCallIdLen && (msg->callIdLen > sip_eval_config->maxCallIdLen))
	{
		ALERT(SIP_EVENT_BAD_CALL_ID,SIP_EVENT_BAD_CALL_ID_STR);
		ret = SIP_FAILURE;
	}

	if(msg->cseqnum > MAX_NUM_32BIT)
	{
		ALERT(SIP_EVENT_BAD_CSEQ_NUM,SIP_EVENT_BAD_CSEQ_NUM_STR);
		ret = SIP_FAILURE;
	}
	if ( sip_eval_config->maxRequestNameLen && (msg->cseqNameLen > sip_eval_config->maxRequestNameLen))
	{
		ALERT(SIP_EVENT_BAD_CSEQ_NAME,SIP_EVENT_BAD_CSEQ_NAME_STR);
		ret = SIP_FAILURE;
	}

	/*Alert here after parsing*/
	if(0 == msg->viaLen)
	{
		ALERT(SIP_EVENT_EMPTY_VIA,SIP_EVENT_EMPTY_VIA_STR)
		ret = SIP_FAILURE;
	}
	else if (sip_eval_config->maxViaLen && (msg->viaLen > sip_eval_config->maxViaLen))
	{
		ALERT(SIP_EVENT_BAD_VIA,SIP_EVENT_BAD_VIA_STR);
		ret = SIP_FAILURE;
	}

	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Method flag: %d\n", msg->methodFlag));

	// Contact is required for invite message
	if((0 == msg->contactLen)&&(msg->methodFlag == SIP_METHOD_INVITE)&&(0 == msg->status_code))
	{
		ALERT(SIP_EVENT_EMPTY_CONTACT,SIP_EVENT_EMPTY_CONTACT_STR)
		ret = SIP_FAILURE;
	}
	else if (sip_eval_config->maxContactLen && (msg->contactLen > sip_eval_config->maxContactLen))
	{
		ALERT(SIP_EVENT_BAD_CONTACT,SIP_EVENT_BAD_CONTACT_STR);
		ret = SIP_FAILURE;
	}

	if((0 == msg->contentTypeLen) && (msg->content_len > 0))
	{
	    ALERT(SIP_EVENT_EMPTY_CONTENT_TYPE,SIP_EVENT_EMPTY_CONTENT_TYPE_STR)
	                ret = SIP_FAILURE;
	}

	return ret;
}

/********************************************************************
 * Function: sip_parse_via()
 *
 * Parse the via field: Via can have multiple header
 *
 * Arguments:
 *  SIPMsg *    - sip message
 *  char* start  - start of the via filed line
 *  char* end   - end of the line
 * Returns:
 *  SIP_PARSE_ERROR
 *  SIP_PARSE_SUCCESS
 ********************************************************************/

static int sip_parse_via(SIPMsg *msg, const char *start, const char *end)
{
	int length = end -start;
	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Via value: %.*s\n", length, start););
	msg->viaLen = msg->viaLen + length;
	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Via length: %d\n", msg->viaLen););

#ifdef DUMP_BUFFER
        dumpBuffer(VIA_DUMP,start,msg->viaLen);
#endif

	return SIP_PARSE_SUCCESS;
}
/********************************************************************
 * Function: sip_parse_from()
 *
 * Parse the from field and get from tag
 * Note: From has no multiple header
 *
 * Arguments:
 *  SIPMsg *    - sip message
 *  char* start  - start of the from filed line
 *  char* end   - end of the line
 * Returns:
 *  SIP_PARSE_ERROR
 *  SIP_PARSE_SUCCESS
 ********************************************************************/

static int sip_parse_from(SIPMsg *msg, const char *start, const char *end)
{
	DEBUG_WRAP(int length = end -start;)
	char *buff;
    char *userEnd;
    char *userStart;

	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "From value: %.*s\n", length, start););
	msg->from = (char *)start;
	msg->fromLen = end - start;

	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "From length: %d , content: %.*s\n",
			msg->fromLen, msg->fromLen, msg->from););

#ifdef DUMP_BUFFER
        dumpBuffer(FROM_DUMP,msg->from,msg->fromLen);
#endif

	/*Get the from tag*/
	msg->fromTagLen = 0;

	buff = memchr(start, ';', msg->fromLen);
	while ((NULL != buff)&& (buff < end))
	{
		if (0 == strncmp(buff + 1, SIP_TAG_KEYWORD, SIP_TAG_KEYWORD_LEN))
		{
			msg->from_tag = buff + SIP_TAG_KEYWORD_LEN + 1;
			msg->fromTagLen = end - msg->from_tag;
			msg->dlgID.fromTagHash = strToHash(msg->from_tag,msg->fromTagLen);
			break;
		}
		buff = memchr(buff + 1, ';', msg->fromLen);
	}

#ifdef DUMP_BUFFER
        dumpBuffer(FROM_TAG_DUMP,msg->from_tag,msg->fromTagLen);
#endif
	userStart = memchr(msg->from, ':', msg->fromLen);
	userEnd = memchr(msg->from, '>', msg->fromLen);
    if (userStart && userEnd && (userEnd > userStart))
    {
        /*strndup here */
	    msg->userName = userStart+1;
	    msg->userNameLen = userEnd - userStart - 1;

    }
    else
    {
	    msg->userName = NULL;
	    msg->userNameLen = 0;
    }

#ifdef DUMP_BUFFER
        dumpBuffer(USER_NAME_DUMP,msg->userName,msg->userNameLen);
#endif

	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "From tag length: %d , hash: %u, content: %.*s\n",
			msg->fromTagLen, msg->dlgID.fromTagHash, msg->fromTagLen, msg->from_tag););
	return SIP_PARSE_SUCCESS;
}

/********************************************************************
 * Function: sip_parse_to()
 *
 * Parse the to field and get to tag information
 * Note: To has no multiple header
 *
 * Arguments:
 *  SIPMsg *    - sip message
 *  char* start  - start of the to filed line
 *  char* end   - end of the line
 * Returns:
 *  SIP_PARSE_ERROR
 *  SIP_PARSE_SUCCESS
 ********************************************************************/

static int sip_parse_to(SIPMsg *msg, const char *start, const char *end)
{
	DEBUG_WRAP(int length = end -start;)
	char *buff;
	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "To value: %.*s\n", length, start););
	msg->to = (char *)start;
	msg->toLen = end - start;

	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "To length: %d , content: %.*s\n",
			msg->toLen, msg->toLen, msg->to););

#ifdef DUMP_BUFFER
        dumpBuffer(TO_DUMP,msg->to,msg->toLen);
#endif

	/*Processing tag information*/
	msg->toTagLen = 0;

	buff = memchr(start, ';', msg->toLen);
	while ((NULL != buff)&& (buff < end))
	{
		if (0 == strncmp(buff + 1, SIP_TAG_KEYWORD, SIP_TAG_KEYWORD_LEN))
		{
			msg->to_tag = buff + SIP_TAG_KEYWORD_LEN + 1;
			msg->toTagLen = end - msg->to_tag;
			msg->dlgID.toTagHash = strToHash(msg->to_tag,msg->toTagLen);
			break;
		}
		buff = memchr(buff + 1, ';', msg->toLen);
	}

#ifdef DUMP_BUFFER
        dumpBuffer(TO_TAG_DUMP,msg->to_tag,msg->toTagLen);
#endif

	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "To tag length: %d , Hash: %u, content: %.*s\n",
			msg->toTagLen, msg->dlgID.toTagHash, msg->toTagLen, msg->to_tag););
	return SIP_PARSE_SUCCESS;
}

static inline bool is_valid_ip(const char *start, const char *end)
{
    sfaddr_t ip;
    char ipStr[INET6_ADDRSTRLEN];
    int length = end - start;

    /*Get the IP address*/
    if(length > INET6_ADDRSTRLEN - 1)
    {
        length = INET6_ADDRSTRLEN - 1;
    }
    memcpy(ipStr, start, length);
    ipStr[length] = '\0';

    DEBUG_WRAP(DebugMessage(DEBUG_SIP, "IP data: %s\n", ipStr););

    if( (sfaddr_pton(ipStr, &ip)) != SFIP_SUCCESS)
    {
       DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Not valid IP\n"););
       return false;
    }

    return true;
}

/********************************************************************
 * Function: sip_parse_call_id()
 *
 * Parse the call-id field
 * Note: call-id has no multiple header
 *
 * Arguments:
 *  SIPMsg *    - sip message
 *  char* start  - start of the filed line
 *  char* end   - end of the line
 * Returns:
 *  SIP_PARSE_ERROR
 *  SIP_PARSE_SUCCESS
 ********************************************************************/

static int sip_parse_call_id(SIPMsg *msg, const char *start, const char *end)
{
    char* at;
	int length = end -start;
	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Call-Id value: %.*s\n", length, start););
	msg->call_id = (char *) start;
	/*ignore ip address in call id by adjusting length*/
	at = memchr(start, '@', length);
	if(at && (at < end) && is_valid_ip(at+1, end))
	{
	    length = at - start;
	}

	msg->callIdLen = end - start;

#ifdef DUMP_BUFFER
        dumpBuffer(CALL_ID_DUMP,msg->call_id, length);
#endif

	msg->dlgID.callIdHash =  strToHash(msg->call_id, length);
	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Call-Id length: %d, Hash: %u\n",
        msg->callIdLen, msg->dlgID.callIdHash););

	return SIP_PARSE_SUCCESS;
}

/********************************************************************
 * Function: sip_parse_user_agent()
 *
 * Parse the user_agent field
 *
 * Arguments:
 *  SIPMsg *    - sip message
 *  char* start  - start of the field line
 *  char* end   - end of the line
 * Returns:
 *  SIP_PARSE_ERROR
 *  SIP_PARSE_SUCCESS
 ********************************************************************/
static int sip_parse_user_agent(SIPMsg *msg, const char *start, const char *end)
{
	DEBUG_WRAP(int length = end -start;)
	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "User-Agent value: %.*s\n", length, start););

	msg->userAgent = (char *)start;
	msg->userAgentLen = end - start;

#ifdef DUMP_BUFFER
        dumpBuffer(USER_AGENT_DUMP,msg->userAgent,msg->userAgentLen);
#endif

	return SIP_PARSE_SUCCESS;
}
/********************************************************************
 * Function: sip_parse_server()
 *
 * Parse the server field
 *
 * Arguments:
 *  SIPMsg *    - sip message
 *  char* start  - start of the field line
 *  char* end   - end of the line
 * Returns:
 *  SIP_PARSE_ERROR
 *  SIP_PARSE_SUCCESS
 ********************************************************************/
static int sip_parse_server(SIPMsg *msg, const char *start, const char *end)
{
	DEBUG_WRAP(int length = end -start;)
	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Server value: %.*s\n", length, start););

	msg->server = (char *)start;
	msg->serverLen = end - start;

#ifdef DUMP_BUFFER
        dumpBuffer(SERVER_DUMP,msg->server,msg->serverLen);
#endif

	return SIP_PARSE_SUCCESS;
}

/********************************************************************
 * Function: sip_parse_cseq()
 *
 * Parse the cseq field: get sequence number and request name
 * Note: Cseq has no multiple header
 *
 * Arguments:
 *  SIPMsg *    - sip message
 *  char* start  - start of the filed line
 *  char* end   - end of the line
 * Returns:
 *  SIP_PARSE_ERROR
 *  SIP_PARSE_SUCCESS
 ********************************************************************/

static int sip_parse_cseq(SIPMsg *msg, const char *start, const char *end)
{
	char *next = NULL;
	DEBUG_WRAP(int length = end -start;)
	SIPMethodNode* method = NULL;

	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "CSeq value: %.*s\n", length, start););
	msg->cseqnum = _dpd.SnortStrtoul(start, &next, 10);
	if ((NULL != next )&&(next < end))
	{
		msg->cseqName = next + 1;
		msg->cseqNameLen = end - msg->cseqName;
		method = SIP_FindMethod (sip_eval_config->methods, msg->cseqName, msg->cseqNameLen);
	}
	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "CSeq number: %d, CSeqName: %.*s\n",
			                msg->cseqnum, msg->cseqNameLen, msg->cseqName););

#ifdef DUMP_BUFFER
        dumpBuffer(CSEQ_NAME_DUMP,msg->cseqName,msg->cseqNameLen);
#endif

	if (NULL == method)
	{
		ALERT(SIP_EVENT_INVALID_CSEQ_NAME,SIP_EVENT_INVALID_CSEQ_NAME_STR)
		return SIP_PARSE_ERROR;
	}
	else
	{
		/*Use request name only for response message*/
	    if ((SIP_METHOD_NULL == msg->methodFlag)&&( msg->status_code > 0))
		   msg->methodFlag = method->methodFlag;
	    else if ( method->methodFlag != msg->methodFlag)
	    {
	        ALERT(SIP_EVENT_MISMATCH_METHOD,SIP_EVENT_MISMATCH_METHOD_STR)
	    }
	    DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Found the method: %s, Flag: 0x%x\n", method->methodName, method->methodFlag));

	}

	return SIP_PARSE_SUCCESS;
}

/********************************************************************
 * Function: sip_parse_contact()
 *
 * Parse the to contact field
 * Note: Contact has multiple header
 *
 * Arguments:
 *  SIPMsg *    - sip message
 *  char* start  - start of the filed line
 *  char* end   - end of the line
 * Returns:
 *  SIP_PARSE_ERROR
 *  SIP_PARSE_SUCCESS
 ********************************************************************/

static int sip_parse_contact(SIPMsg *msg, const char *start, const char *end)
{
	int length = end -start;
	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Contact value: %.*s\n", length, start););
	msg->contact = (char *) start;
	msg->contactLen = msg->contactLen + length;
	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Contact length: %d\n", msg->contactLen););

#ifdef DUMP_BUFFER
        dumpBuffer(CONTACT_DUMP,msg->contact,msg->contactLen);
#endif

	return SIP_PARSE_SUCCESS;
}

/********************************************************************
 * Function: sip_parse_authorization()
 *
 * Parse the to authorization field
 *
 * Arguments:
 *  SIPMsg *    - sip message
 *  char* start  - start of the filed line
 *  char* end   - end of the line
 * Returns:
 *  SIP_PARSE_ERROR
 *  SIP_PARSE_SUCCESS
 ********************************************************************/

static int sip_parse_authorization(SIPMsg *msg, const char *start, const char *end)
{
	DEBUG_WRAP(int length = end -start;)
	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Authorization value: %.*s\n", length, start););
	msg->authorization = (char *) start;
	return SIP_PARSE_SUCCESS;
}
/********************************************************************
 * Function: sip_parse_content_type()
 *
 * Parse the to content type field
 *
 * Arguments:
 *  SIPMsg *    - sip message
 *  char* start  - start of the filed line
 *  char* end   - end of the line
 * Returns:
 *  SIP_PARSE_ERROR
 *  SIP_PARSE_SUCCESS
 ********************************************************************/

static int sip_parse_content_type(SIPMsg *msg, const char *start, const char *end)
{
	DEBUG_WRAP(int length = end -start;)
	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Content type value: %.*s\n", length, start););
	msg->contentTypeLen = end - start;
	msg->content_type = (char *) start;
	return SIP_PARSE_SUCCESS;
}
/********************************************************************
 * Function: sip_parse_content_len()
 *
 * Parse the to content length field
 *
 * Arguments:
 *  SIPMsg *    - sip message
 *  char* start  - start of the filed line
 *  char* end   - end of the line
 * Returns:
 *  SIP_PARSE_ERROR
 *  SIP_PARSE_SUCCESS
 ********************************************************************/

static int sip_parse_content_len(SIPMsg *msg, const char *start, const char *end)
{
	char *next = NULL;
#ifdef DEBUG
	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Content length value: %.*s\n", (int)(end - start), start););
#endif

	msg->content_len = _dpd.SnortStrtoul(start, &next, 10);
	if ( sip_eval_config->maxContentLen && (msg->content_len > sip_eval_config->maxContentLen))
		ALERT(SIP_EVENT_BAD_CONTENT_LEN,SIP_EVENT_BAD_CONTENT_LEN_STR);
	/*Check the length of the value*/
	if (next > start + SIP_CONTENT_LEN) // This check is to prevent overflow
	{
	    if (sip_eval_config->maxContentLen)
	        ALERT(SIP_EVENT_BAD_CONTENT_LEN,SIP_EVENT_BAD_CONTENT_LEN_STR);
	    return SIP_PARSE_ERROR;
	}
	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Content length: %u\n", msg->content_len););

	return SIP_PARSE_SUCCESS;
}

/********************************************************************
 * Function: sip_parse_content_encode()
 *
 * Parse the to content encode field
 *
 * Arguments:
 *  SIPMsg *    - sip message
 *  char* start  - start of the filed line
 *  char* end   - end of the line
 * Returns:
 *  SIP_PARSE_ERROR
 *  SIP_PARSE_SUCCESS
 ********************************************************************/

static int sip_parse_content_encode(SIPMsg *msg, const char *start, const char *end)
{
	DEBUG_WRAP(int length = end -start;)
	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Content encode value: %.*s\n", length, start););
	msg->content_encode = (char *) start;
	return SIP_PARSE_SUCCESS;
}
/********************************************************************
 * Function: sip_parse_sdp_o()
 *
 * Parse SDP origination information
 *
 * Arguments:
 *  SIPMsg *    - sip message
 *  char* start  - start of the filed line
 *  char* end   - end of the line
 * Returns:
 *  SIP_PARSE_ERROR
 *  SIP_PARSE_SUCCESS
 ********************************************************************/
static int sip_parse_sdp_o(SIPMsg *msg, const char *start, const char *end)
{
	int length;
	char *spaceIndex = NULL;
	char *spaceIndex2 = NULL;

	if (NULL == msg->mediaSession)
		return SIP_PARSE_ERROR;
	length = end - start;
	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Origination information: %.*s\n", length, start););
	// Get username and session ID information (before second space)
	spaceIndex = memchr(start, ' ', length); // first space
	if ((NULL == spaceIndex)||(spaceIndex == end))
		return SIP_PARSE_ERROR;
	spaceIndex = memchr(spaceIndex + 1, ' ', end - spaceIndex -1 ); // second space
	if (NULL == spaceIndex)
		return SIP_PARSE_ERROR;
	spaceIndex2 = memchr(spaceIndex + 1, ' ', end - spaceIndex -1 ); // third space
	if (NULL == spaceIndex2)
		return SIP_PARSE_ERROR;

	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Session information: %.*s\n", spaceIndex - start, start););
    //sessionId uses all elements from o: line except sessionId version
	msg->mediaSession->sessionID =  strToHash(start, spaceIndex - start);
	msg->mediaSession->sessionID +=  strToHash(spaceIndex2+1, end - (spaceIndex2+1));

	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Session ID: %u\n", msg->mediaSession->sessionID););
	return SIP_PARSE_SUCCESS;
}
/********************************************************************
 * Function: sip_parse_sdp_c()
 *
 * Parse SDP connection data
 *
 * Arguments:
 *  SIPMsg *    - sip message
 *  char* start  - start of the filed line
 *  char* end   - end of the line
 * Returns:
 *  SIP_PARSE_ERROR
 *  SIP_PARSE_SUCCESS
 ********************************************************************/
static int sip_parse_sdp_c(SIPMsg *msg, const char *start, const char *end)
{
	int length;
	sfaddr_t *ip;
	char ipStr[INET6_ADDRSTRLEN];
	char *spaceIndex = NULL;

	if (NULL == msg->mediaSession)
		return SIP_PARSE_ERROR;
	length = end - start;
	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Connection data: %.*s\n", length, start););

	/*Get the IP address*/
	spaceIndex = memchr(start, ' ', length); // first space
	if ((NULL == spaceIndex)||(spaceIndex == end))
		return SIP_PARSE_ERROR;
	spaceIndex = memchr(spaceIndex + 1, ' ', end - spaceIndex -1 ); // second space
	if (NULL == spaceIndex)
		return SIP_PARSE_ERROR;
	length = end - spaceIndex;

	if(length > INET6_ADDRSTRLEN - 1)
    {
    	length = INET6_ADDRSTRLEN - 1;
    }
	memcpy(ipStr, spaceIndex, length);
	ipStr[length] = '\0';
	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "IP data: %s\n", ipStr););

	// If no default session connect information, add it
	if(NULL == msg->mediaSession->medias)
	{
		ip = &(msg->mediaSession->maddress_default);
	}
	else // otherwise, update the latest media data (header of media list)
	{
		ip = &(msg->mediaSession->medias->maddress);
	}
	if( (sfaddr_pton(ipStr, ip)) != SFIP_SUCCESS)
	{
		DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Parsed error! \n"););
		return SIP_PARSE_ERROR;
	}
	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Parsed Connection data: %s\n", sfip_to_str (ip)););

	return SIP_PARSE_SUCCESS;
}
/********************************************************************
 * Function: sip_parse_sdp_c()
 *
 * Parse media type information
 * Note: to make it easier update the media address, media data are added to the header of media list
 * Arguments:
 *  SIPMsg *    - sip message
 *  char* start  - start of the filed line
 *  char* end   - end of the line
 * Returns:
 *  SIP_PARSE_ERROR
 *  SIP_PARSE_SUCCESS
 ********************************************************************/
static int sip_parse_sdp_m(SIPMsg *msg, const char *start, const char *end)
{
	int length;
	char *spaceIndex = NULL;
    char *next;
    SIP_MediaData *mdata;

	if (NULL == msg->mediaSession)
		return SIP_PARSE_ERROR;
	length = end - start;
	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Media information: %.*s\n", length, start););

	spaceIndex = memchr(start, ' ', length); // first space
	if ((NULL == spaceIndex)||(spaceIndex == end))
		return SIP_PARSE_ERROR;
	mdata = (SIP_MediaData *) _dpd.snortAlloc(1, sizeof(SIP_MediaData),
                                                  PP_SIP, PP_MEM_CATEGORY_SESSION);

	if (NULL == mdata)
		return SIP_PARSE_ERROR;

	mdata->mport = (uint16_t) _dpd.SnortStrtoul(spaceIndex + 1, &next, 10);
	if ((NULL != next)&&('/'==next[0]))
		mdata->numPort = (uint8_t)_dpd.SnortStrtoul(spaceIndex + 1, &next, 10);
	// Put
    mdata->nextM = msg->mediaSession->medias;
    mdata->maddress = msg->mediaSession->maddress_default;
    msg->mediaSession->medias = mdata;
    DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Media IP: %s, Media port %u, number of media: %d\n",
                sfip_to_str(&mdata->maddress), mdata->mport, mdata->numPort););
	return SIP_PARSE_SUCCESS;
}


/********************************************************************
 * Function: sip_parse()
 *
 * The main entry for parser: process the sip messages.
 *
 * Arguments:
 *  SIPMsg *    - sip message
 *  char* buff - start of the sip message buffer
 *  char* end   - end of the buffer
 *
 * Returns:
 *  SIP_FAILURE
 *  SIP_SUCCESS
 ********************************************************************/
int sip_parse(SIPMsg *msg, const char *buff, char *end)
{
	char *nextIndex;
	char *start;
	int status;

#ifdef DUMP_BUFFER
        dumpBufferInit();
#endif

	/*Initialize key values*/
	msg->methodFlag = SIP_METHOD_NULL;
	msg->status_code = 0;

	/*Parse the start line*/
	start = (char *) buff;
	nextIndex = NULL;
	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Start parsing...\n"));

	msg->header = (uint8_t *) buff;

	status = sip_startline_parse(msg, start, end, &nextIndex);

	if(SIP_FAILURE == status )
	{
		DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Start line parsing failed...\n"));
		return status;
	}

	/*Parse the headers*/
	start = nextIndex;
	status = sip_headers_parse(msg, start, end, &nextIndex);
	msg->headerLen =  nextIndex - buff;

#ifdef DUMP_BUFFER
        dumpBuffer(HEADER_DUMP, (const char *) msg->header, msg->headerLen);
#endif

        if(SIP_FAILURE == status )
	{
		DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Header parsing failed...\n"));
	}

	status = sip_check_headers(msg);

	if(SIP_FAILURE == status )
	{
		DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Headers validation failed...\n"));
	}

	/*Parse the body*/
	start = nextIndex;
	msg->bodyLen = end - start;
	/* Disable this check for TCP. */
	if((!msg->isTcp)&&(msg->content_len > msg->bodyLen))
		ALERT(SIP_EVENT_MISMATCH_CONTENT_LEN,SIP_EVENT_MISMATCH_CONTENT_LEN_STR);

	if (msg->content_len < msg->bodyLen)
	    status = sip_body_parse(msg, start, start + msg->content_len, &nextIndex);
        else
		status = sip_body_parse(msg, start, end, &nextIndex);

#ifdef DUMP_BUFFER
        dumpBuffer(BODY_DUMP, (const char *) msg->body_data,msg->bodyLen);
#endif

	if(SIP_FAILURE == status )
	{
		DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Headers validation failed...\n"));
	}

	// Find out whether multiple SIP messages in this packet
	/* Disable this check for TCP. */
    if ((!msg->isTcp) && (msg->content_len < msg->bodyLen))
    {
    	if (SIP_SUCCESS == sip_startline_parse(msg, start + msg->content_len, end, &nextIndex))
    	{
    		ALERT(SIP_EVENT_MULTI_MSGS,SIP_EVENT_MULTI_MSGS_STR);
    	}
    	else
    	{
            ALERT(SIP_EVENT_MISMATCH_CONTENT_LEN,SIP_EVENT_MISMATCH_CONTENT_LEN_STR);
    	}
    }
	return status;
}

/********************************************************************
 * Function: sip_freeMsg
 *
 * Frees a sip msg.
 * Media session information will be release if they are not used by dialog.
 *
 * Arguments:
 *  SIPMsg *
 *      The sip message to free.
 *
 * Returns: None
 *
 ********************************************************************/
void sip_freeMsg (SIPMsg *msg)
{

	if (NULL == msg)
        return;
	if (NULL != msg->mediaSession)
	{
	  if (SIP_SESSION_SAVED != msg->mediaSession->savedFlag)
		  sip_freeMediaSession(msg->mediaSession);
	}

}
/********************************************************************
 * Function: sip_freeMediaSession
 *
 * Frees a sip media session
 *
 * Arguments:
 *  SIP_MediaSession *
 *      The media session to free.
 *
 * Returns: None
 *
 ********************************************************************/
void sip_freeMediaSession (SIP_MediaSession *mediaSession)
{
	SIP_MediaData *nextNode;
	SIP_MediaData *curNode = NULL;


	if (NULL != mediaSession)
	{
		curNode = mediaSession->medias;
	}

	while (NULL != curNode)
	{
		DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Clear media ip: %s, port: %d, number of port: %d\n",
				sfip_to_str(&curNode->maddress), curNode->mport, curNode->numPort ));
		nextNode = curNode->nextM;
		_dpd.snortFree(curNode, sizeof(SIP_MediaData), PP_SIP, PP_MEM_CATEGORY_SESSION);
		curNode = nextNode;
	}
	if (NULL != mediaSession)
	    _dpd.snortFree(mediaSession, sizeof(SIP_MediaSession), PP_SIP,
                           PP_MEM_CATEGORY_SESSION);
}
/********************************************************************
 * Function: sip_freeMediaList
 *
 * Frees a sip media session list
 *
 * Arguments:
 *  SIP_MediaList
 *      The media session list to free.
 *
 * Returns: None
 *
 ********************************************************************/
void sip_freeMediaList (SIP_MediaList medias)
{
	SIP_MediaSession *nextNode;
	SIP_MediaSession *curNode = medias;

	while (NULL != curNode)
	{
		DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Clean Media session default IP: %s,  session ID: %u\n",
				sfip_to_str(&curNode->maddress_default), curNode->sessionID));
		nextNode = curNode->nextS;
		sip_freeMediaSession(curNode);
		curNode = nextNode;
	}

}
#endif
