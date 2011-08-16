/****************************************************************************
 *
 * Copyright (C) 2003-2011 Sourcefire, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************/
 
/**
**  @file       hi_client.c
**
**  @author     Daniel Roelker <droelker@sourcefire.com>
**
**  @brief      Main file for all the client functions and inspection
**              flow.
**
**
**  The job of the client module is to analyze and inspect the HTTP
**  protocol, finding where the various fields begin and end.  This must
**  be accomplished in a stateful and stateless manner.
**
**  While the fields are being determined, we also do checks for 
**  normalization, so we don't normalize fields that don't need it.
**
**  Currently, the only fields we check for this is the URI and the
**  parameter fields.
**  
**  NOTES:
**    - 3.8.03:  Initial development.  DJR
**    - 2.4.05:  Added tab_uri_delimiter config option.  AJM.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <errno.h>

#include "hi_ui_config.h"
#include "hi_si.h"
#include "hi_mi.h"
#include "hi_client.h"
#include "hi_eo_log.h"
#include "hi_util.h"
#include "hi_util_hbm.h"
#include "hi_return_codes.h"
#include "util.h"
#include "mstring.h"
#include "sfutil/sf_base64decode.h"
#include "hi_cmd_lookup.h"
#include "detection_util.h"

#define HEADER_NAME__COOKIE "Cookie"
#define HEADER_LENGTH__COOKIE 6
#define HEADER_NAME__CONTENT_LENGTH "Content-length"
#define HEADER_LENGTH__CONTENT_LENGTH 14
#define HEADER_NAME__XFF "X-Forwarded-For"
#define HEADER_LENGTH__XFF 15
#define HEADER_NAME__TRUE_IP "True-Client-IP"
#define HEADER_LENGTH__TRUE_IP 14

/**  This makes passing function arguments much more readable and easier
**  to follow.
*/
typedef int (*LOOKUP_FCN)(HI_SESSION *, const u_char *, const u_char *, const u_char **,
            URI_PTR *);

/*
**  The lookup table contains functions for different HTTP delimiters 
**  (like whitespace and the HTTP delimiter \r and \n).
*/
LOOKUP_FCN lookup_table[256];
int NextNonWhiteSpace(HI_SESSION *Session, const u_char *start,
        const u_char *end, const u_char **ptr, URI_PTR *uri_ptr);

/*
**  NAME
**    CheckChunkEncoding::
*/
/**
**  This routine checks for chunk encoding anomalies in an HTTP client request
**  packet.
**  
**  We convert potential chunk lengths and test them against the user-defined
**  max chunk length.  We log events on any chunk lengths that are over this
**  defined chunk lengths.
**  
**  Chunks are skipped to save time when the chunk is contained in the packet.
**  
**  We assume coming into this function that we are pointed at the beginning
**  of what may be a chunk length.  That's why the iCheckChunk var is set
**  to 1.
**  
**  @param Session pointer to the Session construct
**  @param start   pointer to where to beginning of buffer
**  @param end     pointer to the end of buffer
**  
**  @return integer
**  
**  @retval HI_SUCCESS      function successful
**  @retval HI_INVALID_ARG  invalid argument
*/
int CheckChunkEncoding(HI_SESSION *Session, const u_char *start, const u_char *end, 
        const u_char **post_end, u_char *iChunkBuf, int max_size, 
        int last_chunk_size, int *chunkSize, int *chunkRead)
{
    int   iChunkLen   = 0;
    int    iChunkChars = 0;
    int chunkPresent = 0;
    int    iCheckChunk = 1;
    const u_char *ptr;
    const u_char *jump_ptr;
    int iDataLen = 0;
    int chunkBytesCopied = 0;

    if(!start || !end)
        return HI_INVALID_ARG;

    ptr = start;

    if(last_chunk_size)
    {
        if(last_chunk_size > max_size)
        {
            if(chunkSize)
                *chunkSize = last_chunk_size - max_size ;
            last_chunk_size = max_size;
        }

        iDataLen = end - ptr;

        if(last_chunk_size > iDataLen)
        {
            if(chunkSize)
                *chunkSize = last_chunk_size - iDataLen ;
            last_chunk_size = iDataLen;
        }

        jump_ptr = ptr + last_chunk_size - 1;

        if(hi_util_in_bounds(start, end, jump_ptr))
        {
            chunkPresent = 1;
            if(iChunkBuf)
            {
                memcpy(iChunkBuf, ptr, last_chunk_size);
                chunkBytesCopied = last_chunk_size;
            }
            ptr = jump_ptr + 1;
        }

    }

    while(hi_util_in_bounds(start, end, ptr))
    {
        if(*ptr == '\n' || *ptr == ' ' || *ptr == '\t')
        {
            if(iCheckChunk && iChunkLen != 0)
            {
                if(Session->server_conf->chunk_length < (unsigned int)iChunkLen &&
                   hi_eo_generate_event(Session, HI_EO_CLIENT_LARGE_CHUNK))
                {
                    hi_eo_client_event_log(Session, HI_EO_CLIENT_LARGE_CHUNK,
                                           NULL, NULL);
                }

                SkipBlankAndNewLine(start,end, &ptr);

                if(*ptr == '\n')
                    ptr++;

                iDataLen = end - ptr ;

                if( iChunkLen > iDataLen)
                {
                    if(chunkSize)
                        *chunkSize = iChunkLen - iDataLen;
                    iChunkLen = iDataLen;
                    jump_ptr = ptr + iDataLen - 1;
                }
                else
                {
                    jump_ptr = ptr + iChunkLen;
                }

                if(jump_ptr <= ptr)
                {
                    break;
                }

                if(hi_util_in_bounds(start, end, jump_ptr))
                {
                    chunkPresent = 1;
                    if(iChunkBuf && ((chunkBytesCopied + iChunkLen) <= max_size))
                    {
                        memcpy(iChunkBuf+chunkBytesCopied, ptr, iChunkLen);
                        chunkBytesCopied += iChunkLen;
                    }
                    ptr = jump_ptr;
                    /* Check to see if the chunks ends */
                    if( ((ptr + 2) < end) && (*(ptr + 2) != '\n') && 
                            hi_eo_generate_event(Session, HI_EO_CLIENT_CHUNK_SIZE_MISMATCH))
                    {
                        hi_eo_client_event_log(Session, HI_EO_CLIENT_CHUNK_SIZE_MISMATCH,
                                NULL, NULL);
                    }
                }
                else
                {
                    /*
                    **  Chunk too large for packet, so we bail
                    */
                    break;
                }
            }

            /*
            **  If we've already evaluated the chunk, or we have a valid delimiter
            **  for handling new chunks, we reset and starting evaluating possible
            **  chunk lengths.
            */
            if(iCheckChunk || *ptr == '\n')
            {
                iCheckChunk = 1;
                iChunkLen   = 0;
                iChunkChars = 0;
            }

            ptr++;
            continue;
        }

        if(iCheckChunk)
        {
            if(valid_lookup[*ptr] != HEX_VAL)
            {
                if(*ptr == '\r')
                {
                    ptr++;

                    if(!hi_util_in_bounds(start, end, ptr))
                        break;

                    if(*ptr == '\n')
                        continue;
                }
                else if(*ptr == ';')
                {
                    /*
                    **  This is where we skip through the chunk name=value
                    **  field.
                    */
                    ptr++;

                    while(hi_util_in_bounds(start, end, ptr))
                    {
                        if(*ptr == '\n')
                            break;

                        ptr++;
                    }

                    continue;
                }

                iCheckChunk = 0;
                iChunkLen   = 0;
                iChunkChars = 0;
            }
            else
            {
                if(iChunkChars >= 8)
                {
                    iCheckChunk = 0;
                    iChunkLen   = 0;
                    iChunkChars = 0;
                }
                else
                {
                    iChunkLen <<= 4;
                    iChunkLen |= (unsigned int)(hex_lookup[*ptr]);
                    iChunkChars++;
                }
            }
        }

        ptr++;
    }
    if (chunkPresent )
    {
        if(post_end)
        {
            *(post_end) = ptr;
        }

        if(chunkRead)
        {
            *chunkRead = chunkBytesCopied;
        }
        return 1;
    }

    return HI_SUCCESS;
}

/*
**  NAME
**    FindPipelineReq::
*/
/**
**  Catch multiple requests per packet, by returning pointer to after the
**  end of the request header if there is another request.
**  
**  There are 4 types of "valid" delimiters that we look for.  They are:
**  "\r\n\r\n"
**  "\r\n\n"
**  "\n\r\n"
**  "\n\n"
**  The only patterns that we really only need to look for are:
**  "\n\r\n"
**  "\n\n"
**  The reason being that these two patterns are suffixes of the other 
**  patterns.  So once we find those, we are all good.
**  
**  @param Session pointer to the session
**  @param start pointer to the start of text
**  @param end   pointer to the end of text
**  
**  @return pointer
**  
**  @retval NULL  Did not find pipeline request
**  @retval !NULL Found another possible request.
*/
static INLINE const u_char *FindPipelineReq(HI_SESSION *Session, 
            const u_char *start, const u_char *end)
{
    const u_char *p;
    u_char *offset;

    if(!start || !end)
        return NULL;

    p = start;

    offset = (u_char*)p;

    /*
    **  We say end - 6 because we need at least six bytes to verify that
    **  there is an end to the URI and still a request afterwards.  To be
    **  exact, we should only subtract 1, but we are not interested in a
    **  1 byte method, uri, etc.
    **
    **  a.k.a there needs to be data after the initial request to inspect
    **  to make it worth our while.
    */ 
    while(p < (end - 6))
    {
        if(*p == '\n')
        {
            if(hi_eo_generate_event(Session, Session->server_conf->max_hdr_len)
               && ((p - offset) >= Session->server_conf->max_hdr_len))
            {
                hi_eo_client_event_log(Session, HI_EO_CLIENT_LONG_HDR, NULL, NULL);
            }

            p++;

            offset = (u_char*)p;

            if(*p < 0x0E)
            {
                if(*p == '\r')
                {
                    p++;

                    if(*p == '\n')
                    {
                        return ++p;
                    }
                }
                else if(*p == '\n')
                {
                    return ++p;
                }
            }
        }

        p++;
    }

    /* Never observed an end-of-field.  Maybe it's not there, but the header is long anyway: */
    if(hi_eo_generate_event(Session, Session->server_conf->max_hdr_len)
       && ((p - start) >= Session->server_conf->max_hdr_len))
    {
        hi_eo_client_event_log(Session, HI_EO_CLIENT_LONG_HDR, NULL, NULL);
    }

    return NULL;
}

/*
**  NAME
**    IsHttpVersion::
*/
/**
**  This checks that there is a version following a space with in an HTTP
**  packet.
**  
**  This function gets called when a whitespace area has ended, and we want
**  to know if a version identifier is followed directly after.  So we look
**  for the rfc standard "HTTP/" and report appropriately.  We also need
**  to make sure that the function succeeds given an end of buffer, so for
**  instance if the buffer ends like "  HTT", we still assume that this is
**  a valid version identifier because of TCP segmentation.
**  
**  We also check for the 0.9 standard of GET URI\r\n.  When we see a \r or
**  a \n, then we just return with the pointer still pointing to that char.
**  The reason is because on the next loop, we'll do the evaluation that
**  we normally do and finish up processing there.
**  
**  @param start pointer to the start of the version identifier
**  @param end   pointer to the end of the buffer (could be the end of the
**               data section, or just to the beginning of the delimiter.
**  
**  @return integer
**  
**  @retval 1 this is an HTTP version identifier
**  @retval 0 this is not an HTTP identifier, or bad parameters
*/
int IsHttpVersion(const u_char **ptr, const u_char *end)
{
    static u_char s_acHttpDelimiter[] = "HTTP/";
    static int    s_iHttpDelimiterLen = 5;
    int    len;
    int    iCtr;

    if(*ptr >= end)
    {
        return 0;
    }

    len = end - *ptr;
    if(len > s_iHttpDelimiterLen)
    {
        len = s_iHttpDelimiterLen;
    }

    /*
    **  This is where we check for the defunct method again.  This method
    **  allows a request of "GET   /index.html    \r[\n]".  So we need to
    **  check validate this as a legal identifier.
    */
    if(**ptr == '\n' || **ptr == '\r')
    {
        /*
        **  We don't increment the pointer because we check for a legal
        **  identifier in the delimiter checking.  Read the comments for
        **  setting the defunct variable in these functions.
        */
        return 1;
    }

    for(iCtr = 0; iCtr < len; iCtr++)
    {
        if(s_acHttpDelimiter[iCtr] != (u_char)toupper((int)**ptr))
        {
            return 0;
        }

        (*ptr)++;
    }     

    /*
    **  This means that we match all the chars that we could given the 
    **  remaining length so we should increment the pointer by that much
    **  since we don't need to inspect this again.
    */

    /* This pointer is not used again.   When 1 is returned it causes 
     * NextNonWhiteSpace to return also.  */
#if 0
    (*ptr)++;
#endif

    return 1;
}

/*
**  NAME
**    find_rfc_delimiter::
*/
/**
**  Check for standard RFC HTTP delimiter.
**
**  If we find the delimiter, we return that URI_PTR structures should
**  be checked, which bails us out of the loop.  If there isn't a RFC
**  delimiter, then we bail with a no URI.  Otherwise, we check for out
**  of bounds.
**
**  @param ServerConf pointer to the server configuration
**  @param start      pointer to the start of payload
**  @param end        pointer to the end of the payload
**  @param ptr        pointer to the pointer of the current index
**  @param uri_ptr    pointer to the URI_PTR construct
**  
**  @return integer
**  
**  @retval HI_OUT_OF_BOUNDS 
**  @retval URI_END end of the URI is found, check URI_PTR.
**  @retval NO_URI  malformed delimiter, no URI.
*/
int find_rfc_delimiter(HI_SESSION *Session, const u_char *start, 
        const u_char *end, const u_char **ptr, URI_PTR *uri_ptr)
{
    if(*ptr == start || !uri_ptr->uri)
        return NO_URI;

    /*
    **  This is important to catch the defunct way of getting URIs without
    **  specifying "HTTP/major.minor\r\n\r\n".  This is a quick way for
    **  us to tell if we are in that state.
    **
    **  We check for a legal identifier to deal with the case of
    **  "some_of_the_uri_in segmented packet \r\n" in the defunct case.
    **  Since we find a "valid" (still defunct) delimiter, we account for
    **  it here, so that we don't set the uri_end to the delimiter.
    **
    **  NOTE:
    **  We now assume that the defunct method is in effect and if there is
    **  a valid identifier, then we don't update the uri_end because it's
    **  already been set when the identifier was validated.
    */

    (*ptr)++;
    if(!hi_util_in_bounds(start, end, *ptr))
    {
        return HI_OUT_OF_BOUNDS;
    }
        
    if(**ptr == '\n')
    {
        uri_ptr->delimiter = (*ptr)-1;

        if(!uri_ptr->ident)
            uri_ptr->uri_end = uri_ptr->delimiter;
        
        return URI_END;
    }

    return NextNonWhiteSpace(Session, start, end, ptr, uri_ptr);
}

/*
**  NAME
**    find_non_rfc_delimiter::
*/
/**
**  Check for non standard delimiter '\n'.
**
**  It now appears that apache and iis both take this non-standard 
**  delimiter.  So, we most likely will always look for it, but maybe
**  give off a special alert or something.
**
**  @param ServerConf pointer to the server configuration
**  @param start      pointer to the start of payload
**  @param end        pointer to the end of the payload
**  @param ptr        pointer to the pointer of the current index
**  @param uri_ptr    pointer to the URI_PTR construct  
**  
**  @return integer
**  
**  @retval URI_END delimiter found, end of URI
**  @retval NO_URI  
*/
int find_non_rfc_delimiter(HI_SESSION *Session, const u_char *start,
        const u_char *end, const u_char **ptr, URI_PTR *uri_ptr)
{
    HTTPINSPECT_CONF *ServerConf = Session->server_conf;

    if(*ptr == start || !uri_ptr->uri)
        return NO_URI;

    /*
    **  This is important to catch the defunct way of getting URIs without
    **  specifying "HTTP/major.minor\r\n\r\n".  This is a quick way for
    **  us to tell if we are in that state.
    **
    **  We check for a legal identifier to deal with the case of
    **  "some_of_the_uri_in segmented packet \r\n" in the defunct case.
    **  Since we find a "valid" (still defunct) delimiter, we account for
    **  it here, so that we don't set the uri_end to the delimiter.
    **
    **  NOTE:
    **  We now assume that the defunct method is in effect and if there is
    **  a valid identifier, then we don't update the uri_end because it's
    **  already been set when the identifier was validated.
    */
    if(ServerConf->iis_delimiter.on)
    {
        if(hi_eo_generate_event(Session, ServerConf->iis_delimiter.alert))
        {
            hi_eo_client_event_log(Session, HI_EO_CLIENT_IIS_DELIMITER,
                                   NULL, NULL);
        }

        uri_ptr->delimiter = *ptr;
        
        if(!uri_ptr->ident)
            uri_ptr->uri_end = uri_ptr->delimiter;
        
        return URI_END;
    }

    /*
    **  This allows us to do something if the delimiter check is not turned
    **  on.  Most likely this is worthy of an alert, IF it's not normal to
    **  see these requests.
    **
    **  But for now, we always return true.
    */
    uri_ptr->delimiter = *ptr;

    if(!uri_ptr->ident)
        uri_ptr->uri_end = uri_ptr->delimiter;

    return URI_END;
}

/*
**  NAME
**    NextNonWhiteSpace::
*/
/**
**  Update the URI_PTR fields spaces, find the next non-white space char,
**  and validate the HTTP version identifier after the spaces.
**  
**  This is the main part of the URI algorithm.  This verifies that there
**  isn't too many spaces in the data to be a URI, it checks that after the
**  second space that there is an HTTP identifier or otherwise it's no good.
**  Also, if we've found an identifier after the first whitespace, and
**  find another whitespace, there is no URI.
**  
**  The uri and uri_end pointers are updated in this function depending
**  on what space we are at, and if the space was followed by the HTTP
**  identifier.  (NOTE:  the HTTP delimiter is no longer "HTTP/", but
**  can also be "\r\n", "\n", or "\r".  This is the defunct method, and
**  we deal with it in the IsHttpVersion and delimiter functions.)
**  
**  @param ServerConf pointer to the server configuration
**  @param start      pointer to the start of payload
**  @param end        pointer to the end of the payload
**  @param ptr        pointer to the pointer of the current index
**  @param uri_ptr    pointer to the URI_PTR construct  
**  
**  @return integer
**  
**  @retval HI_SUCCESS       found the next non-whitespace
**  @retval HI_OUT_OF_BOUNDS whitespace to the end of the buffer
**  @retval URI_END          delimiter found, end of URI
**  @retval NO_URI
*/
int NextNonWhiteSpace(HI_SESSION *Session, const u_char *start, 
        const u_char *end, const u_char **ptr, URI_PTR *uri_ptr)
{
    HTTPINSPECT_CONF *ServerConf = Session->server_conf;
    const u_char **start_sp;
    const u_char **end_sp;

    /*
    **  Horizontal tab is only accepted by apache web servers, not IIS.
    **  Some IIS exploits contain a tab (0x09) in the URI, so we don't want
    **  to treat it as a URI delimiter and cut off the URI.
    */
    if ( **ptr == '\t' && !ServerConf->tab_uri_delimiter )
    {
        (*ptr)++;
        return HI_SUCCESS;
    }

    /*
    **  Reset the identifier, because we've just seen another space.  We
    **  should only see the identifier immediately after a space followed
    **  by a delimiter.
    */
    if(uri_ptr->ident)
    {
        if(ServerConf->non_strict)
        {
            /*
            **  In non-strict mode it is ok to see spaces after the
            **  "identifier", so we just increment the ptr and return.
            */
            (*ptr)++;
            return HI_SUCCESS;
        }
        else
        {
            /*
            **  This means that we've already seen a space and a version
            **  identifier, and now that we've seen another space, we know
            **  that this can't be the URI so we just bail out with no
            **  URI.
            */
            return NO_URI;
        }
    }

    uri_ptr->ident = NULL;

    /*
    **  We only check for one here, because both should be set if one
    **  is.
    */
    if(uri_ptr->first_sp_end)
    {
        /*
        **  If the second space has been set, then this means that we have   
        **  seen a third space, which we shouldn't see in the URI so we
        **  are now done and know there is no URI in this packet.
        */
        if(uri_ptr->second_sp_end)
        {
            return NO_URI;
        }

        /* 
        **  Treat whitespace differently at the end of the URI than we did
        **  at the beginning.  Ignore and return if special characters are
        **  not defined as whitespace after the URI.
        */
        if(ServerConf->whitespace[**ptr]
            && !(ServerConf->whitespace[**ptr] & HI_UI_CONFIG_WS_AFTER_URI))
        {
            (*ptr)++;
            return HI_SUCCESS;
        }

        /*
        **  Since we've seen the second space, we need to update the uri ptr
        **  to the end of the first space, since the URI cannot be before the
        **  first space.
        */
        uri_ptr->uri = uri_ptr->first_sp_end;

        uri_ptr->second_sp_start = *ptr;
        uri_ptr->second_sp_end = NULL;

        start_sp = &uri_ptr->second_sp_start;
        end_sp = &uri_ptr->second_sp_end;
    }
    else
    {
        /*
        **  This means that there is whitespace at the beginning of the line
        **  and we unset the URI so we can set it later if need be.
        **
        **  This is mainly so we handle data that is all spaces correctly.
        **  
        **  In the normal case where we've seen text and then the first space,
        **  we leave the uri ptr pointing at the beginning of the data, and
        **  set the uri end after we've determined where to put it.
        */
        if(start == *ptr)
            uri_ptr->uri = NULL;


        uri_ptr->first_sp_start = *ptr;
        uri_ptr->first_sp_end = NULL;

        start_sp = &uri_ptr->first_sp_start;
        end_sp = &uri_ptr->first_sp_end;
    }

    while(hi_util_in_bounds(start, end, *ptr))
    {
        /*
        **  Check for whitespace
        */
        if(**ptr == ' ')
        {
            (*ptr)++;
            continue;
        }
        else if(ServerConf->whitespace[**ptr])
        {
            if(ServerConf->apache_whitespace.on)
            {
                if(hi_eo_generate_event(Session, 
                                        ServerConf->apache_whitespace.alert))
                {
                    hi_eo_client_event_log(Session, HI_EO_CLIENT_APACHE_WS,
                                           NULL, NULL);
                }
            }
            (*ptr)++;
            continue;
        }
        else
        {
            /*
            **  This sets the sp_end for whatever space delimiter we are on,
            **  whether that is the first space or the second space.
            */
            *end_sp = *ptr;

            if(!IsHttpVersion(ptr, end))
            {
                /*
                **  This is the default method and what we've been doing
                **  since the start of development.
                */
                if(uri_ptr->second_sp_start)
                {
                    /*
                    **  There is no HTTP version indentifier at the beginning
                    **  of the second space, and this means that there is no
                    **  URI.
                    */
                    if(ServerConf->non_strict)
                    {
                        /*
                        **  In non-strict mode, we must assume the URI is
                        **  between the first and second space, so now
                        **  that we've seen the second space that's the
                        **  identifier.
                        */
                        uri_ptr->ident  = *end_sp;
                        uri_ptr->uri_end = *start_sp;

                        return HI_SUCCESS;
                    }
                    else
                    {
                        /*
                        **  Since we are in strict mode here, it means that
                        **  we haven't seen a valid identifier, so there was
                        **  no URI.
                        */

                        return NO_URI;
                    }
                }

                /*
                **  RESET NECESSARY URI_PTRs HERE.  This is the place where
                **  the uri is updated.  It can only happen once, so do it
                **  right here.
                **
                **  When we get here it means that we have found the end of
                **  the FIRST whitespace, and that there was no delimiter,
                **  so we reset the uri pointers and other related 
                **  pointers.
                */
                uri_ptr->uri      = *end_sp;
                uri_ptr->uri_end  = end;
                uri_ptr->norm     = NULL;
                uri_ptr->last_dir = NULL;
                uri_ptr->param    = NULL;
                uri_ptr->proxy    = NULL;
            }
            else
            {
                /*
                **  Means we found the HTTP version identifier and we reset
                **  the uri_end pointer to point to the beginning of the
                **  whitespace detected.
                **
                **  This works for both "uri_is_here HTTP/1.0" and
                **  "METHOD uri_is_here HTTP/1.0", so it works when the
                **  identifier is after either the first or the second
                **  whitespace.
                */
                uri_ptr->ident   = *end_sp;
                uri_ptr->uri_end = *start_sp;
            }

            /*
            **  We found a non-whitespace char
            */
            return HI_SUCCESS;
        }
    }

    /*
    **  This is the case where we've seen text and found a whitespace until
    **  the end of the buffer.  In that case, we set the uri_end to the
    **  beginning of the whitespace.
    */
    uri_ptr->uri_end = *start_sp;

    return HI_OUT_OF_BOUNDS;
}

/*
**  NAME
**    SetPercentNorm::
*/
/**
**  Check for percent normalization in the URI buffer.
**  
**  We don't do much here besides check the configuration, set the pointer,
**  and continue processing.
**
**  @param ServerConf pointer to the server configuration
**  @param start      pointer to the start of payload
**  @param end        pointer to the end of the payload
**  @param ptr        pointer to the pointer of the current index
**  @param uri_ptr    pointer to the URI_PTR construct  
**  
**  @return integer
**  
**  @retval HI_SUCCESS function successful
*/
int SetPercentNorm(HI_SESSION *Session, const u_char *start,
        const u_char *end, const u_char **ptr, URI_PTR *uri_ptr)
{
    HTTPINSPECT_CONF *ServerConf = Session->server_conf;

    if(!uri_ptr->norm && !uri_ptr->ident)
    {
        if(ServerConf->ascii.on)
        {
            uri_ptr->norm = *ptr;
        }
    }

    (*ptr)++;
   
    return HI_SUCCESS;
}

/*
**  NAME
**    CheckLongDir::
*/
/**
**  We check the directory length against the global config.
**  
**  @param Session pointer to the current session
**  @param uri_ptr pointer to the URI state
**  @param ptr     pointer to the current index in buffer
**  
**  @return integer
**  
**  @retval HI_SUCCESS
*/
static INLINE int CheckLongDir(HI_SESSION *Session, URI_PTR *uri_ptr, 
                               const u_char *ptr)
{
    int iDirLen;

    /*
    **  Check for oversize directory
    */
    if(Session->server_conf->long_dir && 
        uri_ptr->last_dir && !uri_ptr->param)
    {
        iDirLen = ptr - uri_ptr->last_dir;

        if(iDirLen > Session->server_conf->long_dir &&
           hi_eo_generate_event(Session, HI_EO_CLIENT_OVERSIZE_DIR))
        {
            hi_eo_client_event_log(Session, HI_EO_CLIENT_OVERSIZE_DIR,
                                   NULL, NULL);
        }
    }

    return HI_SUCCESS;

}

/*
**  NAME
**    SetSlashNorm::
*/
/**
**  Check for any directory traversal or multi-slash normalization.
**  
**  @param ServerConf pointer to the server configuration
**  @param start      pointer to the start of payload
**  @param end        pointer to the end of the payload
**  @param ptr        pointer to the pointer of the current index
**  @param uri_ptr    pointer to the URI_PTR construct  
**  
**  @return integer
**  
**  @retval HI_SUCCESS       function successful
**  @retval HI_OUT_OF_BOUNDS reached the end of the buffer
*/
int SetSlashNorm(HI_SESSION *Session, const u_char *start,
        const u_char *end, const u_char **ptr, URI_PTR *uri_ptr)
{
    HTTPINSPECT_CONF *ServerConf = Session->server_conf;

    CheckLongDir(Session, uri_ptr, *ptr);
    uri_ptr->last_dir = *ptr;

    if(!uri_ptr->norm && !uri_ptr->ident)
    {

        uri_ptr->norm = *ptr;

        (*ptr)++;

        if(!hi_util_in_bounds(start,end, *ptr))
        {
            /*
            **  This is the case where there is a slash as the last char
            **  and we don't want to normalize that since there really
            **  is nothing to normalize.
            */
            uri_ptr->norm = NULL;
            return HI_OUT_OF_BOUNDS;
        }

        /*
        **  Check for directory traversals
        */
        if(ServerConf->directory.on)
        {
            if(**ptr == '.')
            {
                (*ptr)++;
                if(!hi_util_in_bounds(start, end, *ptr))
                {
                    uri_ptr->norm = NULL;
                    return HI_OUT_OF_BOUNDS;
                }

                if(**ptr == '.' || ** ptr == '/')
                {
                    return HI_SUCCESS;
                }
            }
        }

        /*
        **  Check for multiple slash normalization
        */
        if(ServerConf->multiple_slash.on)
        {
            if(**ptr == '/')
            {
                return HI_SUCCESS;
            }
        }

        uri_ptr->norm = NULL;
        return HI_SUCCESS;
    }

    (*ptr)++;

    return HI_SUCCESS;
}

/*
**  NAME
**    SetBackSlashNorm::
*/
/**
**  Check for backslashes and if we need to normalize.
**  
**  This really just checks the configuration option, and sets the norm
**  variable if applicable.
**  
**  @param ServerConf pointer to the server configuration
**  @param start      pointer to the start of payload
**  @param end        pointer to the end of the payload
**  @param ptr        pointer to the pointer of the current index
**  @param uri_ptr    pointer to the URI_PTR construct  
**  
**  @return integer
**  
**  @retval HI_SUCCESS       function successful
*/
int SetBackSlashNorm(HI_SESSION *Session, const u_char *start,
        const u_char *end, const u_char **ptr, URI_PTR *uri_ptr)
{
    HTTPINSPECT_CONF *ServerConf = Session->server_conf;

    if(!uri_ptr->norm && !uri_ptr->ident)
    {
        if(ServerConf->iis_backslash.on)
        {
            uri_ptr->norm = *ptr;
        }
    }

    (*ptr)++;

    return HI_SUCCESS;
}

/*
 * **  NAME
 * **    SetPlusNorm::
 * */
/**
 * **  Check for "+" and if we need to normalize.
 * **  
 * **  
 * **  @param ServerConf pointer to the server configuration
 * **  @param start      pointer to the start of payload
 * **  @param end        pointer to the end of the payload
 * **  @param ptr        pointer to the pointer of the current index
 * **  @param uri_ptr    pointer to the URI_PTR construct  
 * **  
 * **  @return integer
 * **  
 * **  @retval HI_SUCCESS       function successful
 * */


int SetPlusNorm(HI_SESSION *Session, const u_char *start,
        const u_char *end, const u_char **ptr, URI_PTR *uri_ptr)
{
    if(!uri_ptr->norm && !uri_ptr->ident)
    {
        uri_ptr->norm = *ptr;
    }

    (*ptr)++;

    return HI_SUCCESS;
}

/*
**  NAME
**    SetBinaryNorm::
*/
/**
**  Look for non-ASCII chars in the URI.
**  
**  We look for these chars in the URI and set the normalization variable
**  if it's not already set.  I think we really only need this for IIS
**  servers, but we may want to know if it's in the URI too.
**  
**  @param ServerConf pointer to the server configuration
**  @param start      pointer to the start of payload
**  @param end        pointer to the end of the payload
**  @param ptr        pointer to the pointer of the current index
**  @param uri_ptr    pointer to the URI_PTR construct  
**  
**  @return integer
**  
**  @retval HI_SUCCESS       function successful
*/
int SetBinaryNorm(HI_SESSION *Session, const u_char *start,
        const u_char *end, const u_char **ptr, URI_PTR *uri_ptr)
{
    if(!uri_ptr->norm && !uri_ptr->ident)
    {
        uri_ptr->norm = *ptr;
    }

    (*ptr)++;

    return HI_SUCCESS;
}

/*
**  NAME
**    SetParamField::
*/
/**
**  This function sets the parameter field as the first '?'.  The big thing
**  is that we set the param value, so we don't false positive long dir
**  events when it's really just a long parameter field.
**  
**  @param ServerConf pointer to the server configuration
**  @param start      pointer to the start of payload
**  @param end        pointer to the end of the payload
**  @param ptr        pointer to the pointer of the current index
**  @param uri_ptr    pointer to the URI_PTR construct  
**  
**  @return integer
**  
**  @retval HI_SUCCESS       function successful
*/
int SetParamField(HI_SESSION *Session, const u_char *start,
        const u_char *end, const u_char **ptr, URI_PTR *uri_ptr)
{
    if(!uri_ptr->ident)
    {
        uri_ptr->param = *ptr;
    }

    (*ptr)++;

    return HI_SUCCESS;
}
/*
**  NAME
**    SetProxy::
*/
/**
**  This function checks for an absolute URI in the URI.
**  
**  @param ServerConf pointer to the server configuration
**  @param start      pointer to the start of payload
**  @param end        pointer to the end of the payload
**  @param ptr        pointer to the pointer of the current index
**  @param uri_ptr    pointer to the URI_PTR construct  
**  
**  @return integer
**  
**  @retval HI_SUCCESS       function successful
*/
int SetProxy(HI_SESSION *Session, const u_char *start,
        const u_char *end, const u_char **ptr, URI_PTR *uri_ptr)
{
    HTTPINSPECT_CONF *ServerConf = Session->server_conf;

    if(!uri_ptr->ident && !uri_ptr->last_dir)
    {
        if(Session->global_conf->proxy_alert && !ServerConf->allow_proxy)
        {
            if(hi_util_in_bounds(start, end, ((*ptr)+2)))
            {
                if(*((*ptr)+1) == '/' && *((*ptr)+2) == '/')
                {
                    uri_ptr->proxy = *ptr;
                }
            }
        }
    }

    (*ptr)++;

    return HI_SUCCESS;
}

/*
**  NAME
**    SetClientVars::
*/
/**
**  This is where we set the HI_CLIENT values that we found during URI
**  discovery.  This also covers checking these values for errors.
**  
**  @param Client   pointer to HI_CLIENT structure
**  @param uri_ptr  pointer to the uri data
**  
**  @return integer
**  
**  @retval HI_NONFATAL_ERR problem with the uri values.
**  @retval HI_SUCCESS      values set successfully
*/
static int SetClientVars(HI_CLIENT *Client, URI_PTR *uri_ptr, u_int dsize)
{
    /*
    **  We got here either because we found the delimiter or we are
    **  out of bounds.
    */

    /*
    if(uri_ptr->first_sp_start)
        printf("** first_start  = %c\n", *uri_ptr->first_sp_start);
    if(uri_ptr->first_sp_end)
        printf("** first_end    = %c\n", *uri_ptr->first_sp_end);
    if(uri_ptr->second_sp_start)
        printf("** second_start = %c\n", *uri_ptr->second_sp_start);
    if(uri_ptr->second_sp_end)
        printf("** second_end   = %c\n", *uri_ptr->second_sp_end);
    if(uri_ptr->delimiter)
        printf("** delimiter    = %c\n", *uri_ptr->delimiter);
    
    if(uri_ptr->uri)
        printf("** uri          = %c\n", *uri_ptr->uri);
    if(uri_ptr->norm)
        printf("** norm         = %.2x\n", *uri_ptr->norm);
    */

    /*
    **  This means that there was only spaces or delimiters within the 
    **  complete URI.  In this case, there is no valid URI so we just
    **  return such.
    */
    if(uri_ptr->uri == NULL)
    {
        return HI_NONFATAL_ERR;
    }

    /*
    **  This is where we set the Session variables before moving into more
    **  HttpInspect processing.  If we don't get to this point, then we don't
    **  need to set these variables since we would have aborted with a
    **  NONFATAL_ERR.
    */
    Client->request.uri      = uri_ptr->uri;
    Client->request.uri_size = uri_ptr->uri_end - uri_ptr->uri;
    Client->request.uri_norm = uri_ptr->norm;

    /*
    **  LAST RESORT:
    **
    **  This is one of the last checks we do to make sure that we didn't
    **  mess up or anything.
    */
    if(Client->request.uri_size < 1 || Client->request.uri_size > dsize)
    {
        /*
        **  Bad stuff, let's just bail.
        */
        return HI_NONFATAL_ERR;
    }

    /*
    printf("** Norm = %s\n", Client->request.uri_norm ? "YES" : "NO");
    printf("** URI: |%.*s| size = %u\n", Client->request.uri_size,
           Client->request.uri, Client->request.uri_size);
    */

    return HI_SUCCESS;
}

static INLINE int hi_client_extract_post(
    HI_SESSION *Session, HTTPINSPECT_CONF *ServerConf,
    const u_char *ptr, const u_char *end, URI_PTR *result, int content_length)
{
    const u_char *start = ptr;
    const u_char *post_end = end;

    Session->norm_flags &= HI_BODY;

    /* Limit search depth */
    if ((!content_length))
    {
        if ( ServerConf->chunk_length && (CheckChunkEncoding(Session, start, end, &post_end, NULL, 0, 0 , NULL, NULL) == 1) )
        {
            result->uri = start;
            result->uri_end = post_end;
            return POST_END;
        }
        else
        {
            return HI_NONFATAL_ERR;
        }
    }

    if(content_length > 0)
    {
        if ((post_end - ptr ) > content_length)
        {
            post_end = ptr + content_length;
        }
    }
    else
        return HI_NONFATAL_ERR;

    result->uri = start;
    result->uri_end = post_end;

    return POST_END;
}

static INLINE int hi_client_extract_uri(
    HI_SESSION *Session, HTTPINSPECT_CONF *ServerConf, 
    HI_CLIENT * Client, const u_char *start, const u_char *end, 
    const u_char *ptr, URI_PTR *uri_ptr)
{
    int iRet = HI_SUCCESS;

    Session->norm_flags &= ~HI_BODY;

    /*
    **  This loop compares each char to an array of functions
    **  (one for each char) and calling that function if there is one.
    **  
    **  If there is no function, then we just increment the char ptr and
    **  continue processing.
    **
    **  If there is a function, we call that function and process.  It's
    **  important to note that the function that is called is responsible
    **  for incrementing the ptr to the next char to be inspected.  The
    **  loop does not increment the pointer when a function is called to
    **  allow the maximum flexibility to the functions.
    */

    while(hi_util_in_bounds(start, end, ptr))
    {
        if(!ServerConf->extended_ascii_uri)
        {
            /* isascii returns non-zero if it is ascii */
            if (isascii((int)*ptr) == 0)
            {
                /* Possible post data or something else strange... */
                iRet = URI_END;
                break;
            }
        }

        if(lookup_table[*ptr] || ServerConf->whitespace[*ptr])
        {
            if(lookup_table[*ptr])
            {
                iRet = (lookup_table[*ptr])(Session, start, end,
                            &ptr, uri_ptr);
            }
            else
            {
                iRet = NextNonWhiteSpace(Session, start, end, &ptr, uri_ptr);
            }

            if(iRet)
            {
                if(iRet == URI_END)
                {
                    /*
                    **  You found a URI, let's break and check it out.
                    */
                    break;
                }
                else if(iRet == HI_OUT_OF_BOUNDS)
                {
                    /*
                    **  Means you've reached the end of the buffer.  THIS
                    **  DOESN'T MEAN YOU HAVEN'T FOUND A URI.
                    */
                    break;
                }
                else /* NO_URI */
                {
                    /*
                    **  Check for chunk encoding, because the delimiter can
                    **  also be a space, which would look like a pipeline request
                    **  to us if we don't do this first.
                    */
                    if(Session->server_conf->chunk_length)
                            CheckChunkEncoding(Session, start, end, NULL, NULL, 0, 0, NULL, NULL);

                    /*
                    **  We only inspect the packet for another pipeline
                    **  request if there wasn't a previous pipeline request.
                    **  The reason that we do this is because 
                    */  
                    if(!Client->request.pipeline_req)
                    {
                        /*
                        **  Just because there was no URI in the first part
                        **  the packet, doesn't mean that this isn't a
                        **  pipelined request that has been segmented.
                        */
                        if(!ServerConf->no_pipeline)
                        {
                            Client->request.pipeline_req = FindPipelineReq(Session, ptr, end);
                            if(Client->request.pipeline_req)
                            {
                                return HI_SUCCESS;
                            }
                        }
                    }

                    return HI_NONFATAL_ERR;
                }
            }
            else
            {
                /*
                **  This means that we found the next non-whitespace char
                **  and since we are already pointed there, so we just
                **  continue.
                */
                continue;
            }
        }

        ptr++;
    }
    return iRet;
}


const u_char *extract_http_cookie(const u_char *p, const u_char *end, HEADER_PTR *header_ptr, 
        HEADER_FIELD_PTR *header_field_ptr)
{
    const u_char *crlf;
    if (header_ptr->cookie.cookie)
    {
        /* unusal, multiple cookies... alloc new cookie pointer */
        COOKIE_PTR *extra_cookie = calloc(1, sizeof(COOKIE_PTR));
        if (!extra_cookie)
        {
            /* Failure to allocate, stop where we are... */
            header_ptr->header.uri_end = p;
            return p;
        }
        header_field_ptr->cookie->next = extra_cookie;
        header_field_ptr->cookie = extra_cookie;
        /* extra_cookie->next = NULL; */ /* removed, since calloc NULLs this. */
    }
    else
    {
        header_field_ptr->cookie = &header_ptr->cookie;
    }
    header_field_ptr->cookie->cookie = p;                           

    {
        crlf = (u_char *)SnortStrnStr((const char *)p, end - p, "\n");

        /* find a \n  */
        if (crlf) /* && hi_util_in_bounds(start, end, crlf+1)) bounds is checked in SnortStrnStr */
        {
            header_field_ptr->cookie->cookie_end = crlf + 1;
            p = crlf;
        }
        else
        {
            header_ptr->header.uri_end = header_field_ptr->cookie->cookie_end = end;                     
            return end;
        }
    }
    return p;
}


const u_char *extract_http_xff(HI_SESSION *Session, const u_char *p, const u_char *start, 
        const u_char *end, HEADER_PTR *header_ptr, sfip_t **true_ip)
{
    SFIP_RET status;
    sfip_t *tmp;
    char *ipAddr = NULL;
    uint8_t unfold_buf[DECODE_BLEN];
    uint32_t unfold_size =0;
    const u_char *start_ptr, *end_ptr, *cur_ptr;

    if(!true_ip || *true_ip)
        return p;

    SkipBlankSpace(start,end,&p);

    if(hi_util_in_bounds(start, end, p) && *p == ':')
    {
        p++;
        if(sf_unfold_header(p, end-start, unfold_buf, sizeof(unfold_buf), &unfold_size) != 0)
        {
            header_ptr->header.uri_end = end;
            return end;
        }

        p = p + unfold_size;

        start_ptr = unfold_buf; 
        cur_ptr = unfold_buf;
        end_ptr = unfold_buf + unfold_size;
        SkipBlankSpace(start_ptr,end_ptr,&cur_ptr); 
        
        start_ptr = cur_ptr; 
        while( cur_ptr < end_ptr )
        {
            if( *cur_ptr == ' ' || *cur_ptr == '\t' ||
                    *cur_ptr == ',' )
                break;
            cur_ptr++;
        }

        if(cur_ptr - start_ptr)
            ipAddr = SnortStrndup((const char *)start_ptr, cur_ptr - start_ptr );
        if(ipAddr)
        {
            if( (tmp = sfip_alloc(ipAddr, &status)) == NULL )
            {
                if((status != SFIP_ARG_ERR) && (status !=SFIP_ALLOC_ERR))
                {
                    if(hi_eo_generate_event(Session, HI_EO_CLIENT_INVALID_TRUEIP))
                    {
                        hi_eo_client_event_log(Session, HI_EO_CLIENT_INVALID_TRUEIP, NULL, NULL);
                    }
                }
                free(ipAddr);
                return p;
            }
#ifndef SUP_IP6
            if (tmp->family == AF_INET6)
            {
                sfip_free(tmp);
                free(ipAddr);
                return p;
            }
#endif
            *true_ip = tmp;
            free(ipAddr);
        }

    }
    else
    {
        header_ptr->header.uri_end = end;
        return end;
    }

    return p;
    
}

const u_char *extract_http_content_length(HI_SESSION *Session, 
        HTTPINSPECT_CONF *ServerConf, const u_char *p, const u_char *start, 
        const u_char *end, HEADER_PTR *header_ptr, HEADER_FIELD_PTR *header_field_ptr)
{
    const u_char *crlf;
    int space_present = 0;
    if (header_ptr->content_len.cont_len_start)
    {
        if(hi_eo_generate_event(Session, HI_EO_CLIENT_MULTIPLE_CONTLEN))
        {
            hi_eo_client_event_log(Session, HI_EO_CLIENT_MULTIPLE_CONTLEN, NULL, NULL);
        }
        header_ptr->header.uri_end = p;
        header_ptr->content_len.len = 0;
        return p;
    }
    else
    {
        header_field_ptr->content_len = &header_ptr->content_len;
        p = p + 14;
    }
    /* Move past all the blank spaces. Only tabs and spaces are allowed here */
    SkipBlankSpace(start,end,&p);

    if(hi_util_in_bounds(start, end, p) && *p == ':')
    {
        p++;
        if (  hi_util_in_bounds(start, end, p) )
        {
            if ( ServerConf->profile == HI_APACHE || ServerConf->profile == HI_ALL) 
            {
                SkipWhiteSpace(start,end,&p);
            }
            else
            {
                SkipBlankAndNewLine(start,end,&p);
            }
            if( hi_util_in_bounds(start, end, p))
            {
                if ( *p == '\n' )
                {
                    while(hi_util_in_bounds(start, end, p))
                    {
                        if ( *p == '\n')
                        {
                            p++;
                            while( hi_util_in_bounds(start, end, p) && ( *p == ' ' || *p == '\t')) 
                            {
                                space_present = 1;
                                p++;
                            }
                            if ( space_present )
                            {
                                if ( isdigit((int)*p))
                                    break;
                                else if(isspace((int)*p) && 
                                        (ServerConf->profile == HI_APACHE || ServerConf->profile == HI_ALL) )
                                {
                                    SkipWhiteSpace(start,end,&p);
                                }
                                else
                                {   
                                    header_field_ptr->content_len->cont_len_start = 
                                        header_field_ptr->content_len->cont_len_end = NULL;
                                    header_field_ptr->content_len->len = 0;
                                    return p;
                                }
                            }
                            else
                            {
                                header_field_ptr->content_len->cont_len_start = 
                                    header_field_ptr->content_len->cont_len_end = NULL;
                                header_field_ptr->content_len->len = 0;
                                return p;
                            }
                        }
                        else
                          break;
                    }
                }
                else if(!isdigit((int)*p))
                {
                    header_field_ptr->content_len->cont_len_start = 
                        header_field_ptr->content_len->cont_len_end = NULL;
                    header_field_ptr->content_len->len = 0;
                    return p;
                }
                if(isdigit((int)*p))
                {
                    header_field_ptr->content_len->cont_len_start = p;                           
                    p++;
                    while(hi_util_in_bounds(start, end, p))
                    {
                        if(isdigit((int)*p))
                        {
                            p++;
                            continue;
                        }
                        else if((*p == '\n')) /* digit followed by \n */
                        {
                            header_field_ptr->content_len->cont_len_end = p;
                            break;
                        }
                        else if( (!isdigit((int)*p)) && (!isspace((int)*p))) /* alphabet after digit*/
                        {
                            header_field_ptr->content_len->cont_len_start = 
                                header_field_ptr->content_len->cont_len_end = NULL;
                            header_field_ptr->content_len->len = 0;

                            crlf = (u_char *)SnortStrnStr((const char *)p, end - p, "\n");
                            if (crlf)
                            {
                                return p;
                            }
                            else
                            {
                                header_ptr->header.uri_end = end;
                                return end;
                            }
                        } 
                        else
                        {
                            if (ServerConf->profile == HI_APACHE || ServerConf->profile == HI_ALL)
                            {
                                SkipWhiteSpace(start,end,&p);
                            }
                            else
                            {
                                SkipBlankAndNewLine(start,end,&p);
                            }
                            if ( *p == '\n' )
                            {
                                header_field_ptr->content_len->cont_len_end = p;
                                break;
                            }
                            else /*either a "digit digit" or "digit other character" */
                            {
                                header_field_ptr->content_len->cont_len_start = 
                                    header_field_ptr->content_len->cont_len_end = NULL;
                                header_field_ptr->content_len->len = 0;
                                crlf = (u_char *)SnortStrnStr((const char *)p, end - p, "\n");
                                if (crlf)
                                {
                                    p = crlf;
                                    return p;
                                }
                                else
                                {
                                    header_ptr->header.uri_end = end;
                                    return end;
                                }
                            }
                        }
                        p++;
                    }
                }
                else
                {
                    header_field_ptr->content_len->cont_len_start = 
                        header_field_ptr->content_len->cont_len_end = NULL;
                    header_field_ptr->content_len->len = 0;
                    return p;
                }
            }
        }
    }
    else
    {
        if(hi_util_in_bounds(start, end, p))
        {
            crlf = (u_char *)SnortStrnStr((const char *)p, end - p, "\n");
            if(crlf)
            {
                p = crlf;
            }
            else
            {
                header_ptr->header.uri_end = end ;
                return end;
            }
        }
    }
    if ( header_field_ptr && (header_field_ptr->content_len->cont_len_start) &&
      (header_field_ptr->content_len->cont_len_end) )
    {
        char *pcEnd;
        header_field_ptr->content_len->len = SnortStrtol((char *)header_field_ptr->content_len->cont_len_start, &pcEnd, 10);
        if(errno == ERANGE || ((char *)header_field_ptr->content_len->cont_len_start == pcEnd))
        {
            //warning
            header_field_ptr->content_len->len = 0;
        }
    }
    if(!p || !hi_util_in_bounds(start, end, p))
        p = end;

    return p;
}

static INLINE const u_char *extractHeaderFieldValues(HI_SESSION *Session, 
        HTTPINSPECT_CONF *ServerConf, const u_char *p, const u_char *offset, 
        const u_char *start, const u_char *end, HEADER_PTR *header_ptr, HEADER_FIELD_PTR *header_field_ptr, 
        sfip_t **true_ip)
{
    if (((p - offset) == 0) && ((*p == 'C') || (*p == 'c')))
    {
        /* Search for 'Cookie' at beginning, starting from current *p */
        if ( ServerConf->enable_cookie && 
                IsHeaderFieldName(p, end, HEADER_NAME__COOKIE, HEADER_LENGTH__COOKIE))
        {
            p = extract_http_cookie(p, end, header_ptr, header_field_ptr);
        }
        else if ( IsHeaderFieldName(p, end, HEADER_NAME__CONTENT_LENGTH, HEADER_LENGTH__CONTENT_LENGTH) )
        {
            p = extract_http_content_length(Session, ServerConf, p, start, end, header_ptr, header_field_ptr );

        }
    }
    else if (((p - offset) == 0) && ((*p == 'x') || (*p == 'X') || (*p == 't') || (*p == 'T')))
    {
        if ( ServerConf->enable_xff )
        {
            if(IsHeaderFieldName(p, end, HEADER_NAME__XFF, HEADER_LENGTH__XFF))
            {
                p = p + HEADER_LENGTH__XFF;
                p = extract_http_xff(Session, p, start, end, header_ptr, true_ip);
            }
            else if(IsHeaderFieldName(p, end, HEADER_NAME__TRUE_IP, HEADER_LENGTH__TRUE_IP))
            {
                p = p + HEADER_LENGTH__TRUE_IP;
                p = extract_http_xff(Session, p, start, end, header_ptr, true_ip);
            }
        
        }
    }
    return p;
}


/*
**  NAME
**    hi_client_extract_header::
*/
/**
**  Catch multiple requests per packet, by returning pointer to after the
**  end of the request header if there is another request.
**  
**  There are 4 types of "valid" delimiters that we look for.  They are:
**  "\r\n\r\n"
**  "\r\n\n"
**  "\n\r\n"
**  "\n\n"
**  The only patterns that we really only need to look for are:
**  "\n\r\n"
**  "\n\n"
**  The reason being that these two patterns are suffixes of the other 
**  patterns.  So once we find those, we are all good.
**  
**  @param Session pointer to the session
**  @param start pointer to the start of text
**  @param end   pointer to the end of text
**  
**  @return pointer
**  
**  @retval NULL  Did not find pipeline request
**  @retval !NULL Found another possible request.
*/
static INLINE const u_char *hi_client_extract_header(
    HI_SESSION *Session, HTTPINSPECT_CONF *ServerConf, 
    HEADER_PTR *header_ptr, const u_char *start, 
    const u_char *end, sfip_t **true_ip)
{
    int iRet = HI_SUCCESS;
    const u_char *p;
    const u_char *offset;
    URI_PTR version_string;
    HEADER_FIELD_PTR header_field_ptr ;
    int header_count = 0;

    if(!start || !end)
        return NULL;

    p = start;

    /*
    **  We say end - 6 because we need at least six bytes to verify that
    **  there is an end to the URI and still a request afterwards.  To be
    **  exact, we should only subtract 1, but we are not interested in a
    **  1 byte method, uri, etc.
    **
    **  a.k.a there needs to be data after the initial request to inspect
    **  to make it worth our while.
    */ 
    if (p > (end - 6 ))
    {
        header_ptr->header.uri = NULL;
        return p;
    }
    header_ptr->content_len.len = 0;

    header_ptr->header.uri = start;
    header_ptr->header.uri_end = end;

    /* This is to skip past the HTTP/1.0 (or 1.1) version string */
    if (IsHttpVersion(&p, end))
    {
        memset(&version_string, 0, sizeof(URI_PTR));
        version_string.uri = p;

        while (hi_util_in_bounds(start, end, p))
        {
            if(lookup_table[*p] || ServerConf->whitespace[*p])
            {
                if(lookup_table[*p])
                {
                    iRet = (lookup_table[*p])(Session, start, end, &p, &version_string);
                }
                else
                {
                    iRet = NextNonWhiteSpace(Session, start, end, &p, &version_string);
                }

                if(iRet == URI_END)
                {
                    if (*p == '\n')
                    {
                        p++;
                        if (hi_util_in_bounds(start, end, p))
                        {
                            version_string.uri_end = p;
                        }
                        else
                        {
                            return p;
                        }
                    }
                    break;
                }
                else if(iRet == HI_OUT_OF_BOUNDS)
                {
                    return p;
                }
            }
            p++;
        }
        if (iRet == URI_END)
        {
            header_ptr->header.uri = version_string.uri_end + 1;
            offset = (u_char *)p;
        }
        else
        {
            return p;
        }
    }

    offset = (u_char*)p;

    header_ptr->header.uri = p;

    while (hi_util_in_bounds(start, end, p))
    {
        if(*p == '\n')
        {
            header_count++;

            if(hi_eo_generate_event(Session, Session->server_conf->max_hdr_len)
               && ((p - offset) >= Session->server_conf->max_hdr_len))
            {
                hi_eo_client_event_log(Session, HI_EO_CLIENT_LONG_HDR, NULL, NULL);
            }

            if (Session->server_conf->max_headers &&
                (header_count > Session->server_conf->max_headers))
            {
                hi_eo_client_event_log(Session, HI_EO_CLIENT_MAX_HEADERS, NULL, NULL);
            }

            p++;

            offset = (u_char*)p;

            if (!hi_util_in_bounds(start, end, p))
            {
                header_ptr->header.uri_end = p;
                return p;
            }

            /* As performance ugly as this may be, need to bounds check p in each of the
             * if blocks below to prevent read beyond end of buffer */
            if (*p < 0x0E)
            {
                if(*p == '\r')
                {
                    p++;

                    if(hi_util_in_bounds(start, end, p) && (*p == '\n'))
                    {
                        p++;
                        header_ptr->header.uri_end = p;
                        return p;
                    }
                }
                else if(*p == '\n')
                {
                    p++;
                    header_ptr->header.uri_end = p;
                    return p;
                }
            }
            else if ( (p = extractHeaderFieldValues(Session, ServerConf, p, offset, start, end, header_ptr, &header_field_ptr, true_ip)) == end)
            {
                return end;
            }
            
        }
        else if( (p == header_ptr->header.uri) && 
                (p = extractHeaderFieldValues(Session, ServerConf, p, offset, start, end, header_ptr, &header_field_ptr, true_ip)) == end)
        {
            return end;
        }
        if ( *p == '\n') continue;
        p++;
    }

    /* Never observed an end-of-field.  Maybe it's not there, but the header is long anyway: */
    if(hi_eo_generate_event(Session, Session->server_conf->max_hdr_len)
       && ((p - start) >= Session->server_conf->max_hdr_len))
    {
        hi_eo_client_event_log(Session, HI_EO_CLIENT_LONG_HDR, NULL, NULL);
    }

    header_ptr->header.uri_end = p;
    return p;
}

#define CLR_POST(Client) \
    do { \
                Client->request.post_raw = NULL;\
                Client->request.post_raw_size = 0;\
                Client->request.post_norm = NULL; \
    } while(0);

#define CLR_HEADER(Client) \
    do { \
                Client->request.header_raw = NULL;\
                Client->request.header_raw_size = 0;\
                Client->request.header_norm = NULL; \
                Client->request.cookie.cookie = NULL;\
                Client->request.cookie.cookie_end = NULL;\
                Client->request.cookie.next = NULL;\
    } while(0);

/*
**  NAME
**    StatelessInspection::
*/
/**
**  Find the URI and determine whether the URI needs to be normalized.
**  
**  This is a big step in stateless inspection, because we need to reliably
**  find the URI and when possible filter out non-URIs.  We do this using a
**  simple state machine that is based on characters found in the data
**  buffer.
**  
**  Another important aspect of the stateless inspection is the ability to
**  track and inspect pipelined requests.  It is VERY IMPORTANT to reset the
**  pipeline_req pointer, since we don't memset the whole structure.  This
**  pointer is reset in the hi_si_session_inspection() function.  Check there
**  for more details.
**  
**  Normalization is detected when we are looking at the packet for the URI.
**  We look for the following issues:
**      - ////
**      - /../
**      - /./
**      - non-ascii charss
**      - %
**      - \
**  When these things are seen we point to the first occurence in the URI, or
**  where we have to start normalizing.  If the URI is updated to a new
**  pointer, then the normalization pointer is reset and we start over.
**  Using this method should cut down the memcpy()s per URI, since most
**  URIs are not normalized.
**  
**  If this function returns HI_NONFATAL_ERR, we return out of mode_inspection
**  with an error and abort HttpInspect processing, and continue on with
**  any other processing we do.  The Session parameters that we use here are
**  reset in the next time that we do session_inspection, so we don't do
**  any initialization here.
**  
**  @param Session pointer to the HTTP session
**  @param data    pointer to the start of the packet payload
**  @param dsize   size of the payload
**  
**  @return integer
**  
**  @retval HI_INVALID_ARG  invalid argument
**  @retval HI_NONFATAL_ERR no URI detected
**  @retval HI_SUCCESS      URI detected and Session pointers updated
*/
int StatelessInspection(HI_SESSION *Session, const unsigned char *data,
        int dsize, sfip_t **true_ip)
{
    HTTPINSPECT_CONF *ServerConf;
    HTTPINSPECT_CONF *ClientConf;
    HI_CLIENT *Client;
    URI_PTR method_ptr;
    URI_PTR uri_ptr;
    URI_PTR post_ptr;
    HEADER_PTR header_ptr;
    HTTP_CMD_CONF *CmdConf = NULL;
    const u_char *start;
    const u_char *end;
    const u_char *ptr, *mthd;
    const u_char *method_end = NULL;
    int method_len;
    int iRet=0;
    int len;
    char non_ascii_mthd = 0;
    char sans_uri = 0;

    if(!Session || !data || dsize < 1)
    {
        return HI_INVALID_ARG;
    }

    ServerConf = Session->server_conf;
    if(!ServerConf)
    {
        return HI_INVALID_ARG;
    }

    ClientConf = Session->client_conf;
    if(!ClientConf)
    {
        return HI_INVALID_ARG;
    }

    Client = &Session->client;

    memset(&uri_ptr, 0x00, sizeof(URI_PTR));
    memset(&post_ptr, 0x00, sizeof(URI_PTR));
    memset(&header_ptr, 0x00, sizeof(HEADER_PTR));
    memset(&method_ptr, 0x00, sizeof(URI_PTR));

    /*
    **  We set the starting boundary depending on whether this request is
    **  a normal request or a pipeline request.  The end boundary is always
    **  the same whether it is a pipeline request or other.
    */
    if(Client->request.pipeline_req)
    {
        start = Client->request.pipeline_req;
    }
    else
    {
        start = data;
    }

    Client->request.pipeline_req = NULL;

    end = data + dsize;

    ptr = start;

    /*
    **  Apache and IIS strike again . . . Thanks Kanatoko
    **    - Ignore CRLFs at the beginning of the request.
    */
    while(hi_util_in_bounds(start, end, ptr))
    {
        if(*ptr < 0x21)
        {
            if(*ptr < 0x0E && *ptr > 0x08)
            {
                ptr++;
                continue;
            }
            else
            {
                if(*ptr == 0x20)
                {
                    ptr++;
                    continue;
                }
            }
        }
        
        break;
    }

    len = end - ptr;

    mthd = method_ptr.uri = ptr;

    while(hi_util_in_bounds(start, end, mthd))
    {
        if (ServerConf->whitespace[*mthd] || (lookup_table[*mthd] == NextNonWhiteSpace))
        {
            method_end = mthd++;
            break;
        }

        /* isascii returns non-zero if it is ascii */
        if (isascii((int)*mthd) == 0)
        {
            /* Possible post data or something else strange... */
            method_end = mthd++;
            non_ascii_mthd = 1;
            break;
        }

        mthd++;
    }
    if (method_end)
    {
        method_ptr.uri_end = method_end;
    }
    else
    {
        method_ptr.uri_end = end;
    }
    method_len = method_ptr.uri_end - method_ptr.uri;

    /* Need slightly special handling for POST requests 
     * Since we don't normalize on the request method itself,
     * just do a strcmp here and skip the characters below. */
    if(method_len == 4 && !strncasecmp("POST", (const char *)method_ptr.uri, 4)) 
    {
        hi_stats.post++;
        Client->request.method = HI_POST_METHOD;
    }
    else if(method_len == 3 && !strncasecmp("GET", (const char *)method_ptr.uri, 3))
    {
        hi_stats.get++;
        Client->request.method = HI_GET_METHOD;
    }
    else if(method_len > 0 && method_len <= MAX_METHOD_LEN )
    {
        CmdConf = http_cmd_lookup_find(ServerConf->cmd_lookup, (const char *)method_ptr.uri,
                method_len, &iRet);

        if(iRet == -1 || (CmdConf == NULL))
        {
            Client->request.method = HI_UNKNOWN_METHOD;
        }

    }
    else
    {
        Client->request.method = HI_UNKNOWN_METHOD;
    }

    if (Client->request.method == HI_UNKNOWN_METHOD)
    {
        if (IsHttpVersion(&ptr, end))
        {
            sans_uri = 1;
            iRet = URI_END;
            method_ptr.uri = method_ptr.uri_end = NULL;
        }
        else if (method_len == len)
        {
            sans_uri = 1;
            iRet = URI_END;
            method_ptr.uri = method_ptr.uri_end = NULL;
        }
        else if (SnortStrnPbrk((const char *)method_ptr.uri, method_len, "()<>@,;:\\\"/[]?={} \t") != NULL)
        {
            /* Look for the seperator charactors as part of the method */
            sans_uri = 1;
            iRet = URI_END;
            method_ptr.uri = method_ptr.uri_end = NULL;
        }
        else if (non_ascii_mthd == 1)
        {
            sans_uri = 1;
            iRet = URI_END;
            method_ptr.uri = method_ptr.uri_end = NULL;
        }
    }

    if (!sans_uri )
    {
        uri_ptr.uri = ptr;
        uri_ptr.uri_end = end;

        /* This will set up the URI pointers - effectively extracting
         * the URI. */
        iRet = hi_client_extract_uri(
             Session, ServerConf, Client, start, end, ptr, &uri_ptr);
    }

    /* Check if the URI exceeds the max header field length */
    /* Only check if we succesfully observed a GET or POST method, otherwise,
     * this may very well be a POST body */
    if(iRet == URI_END &&
        hi_eo_generate_event(Session, ServerConf->max_hdr_len) &&
         ((uri_ptr.uri_end - uri_ptr.uri) >= ServerConf->max_hdr_len))
    {
        hi_eo_client_event_log(Session, HI_EO_CLIENT_LONG_HDR, NULL, NULL);
    }

    if(iRet == URI_END && 
        !(ServerConf->uri_only) &&
        !(Client->request.method & HI_UNKNOWN_METHOD))
    {
        Client->request.method_raw = method_ptr.uri;
        Client->request.method_size = method_ptr.uri_end - method_ptr.uri;
        ///XXX
        ///Copy out the header into its own buffer...,
        /// set ptr to end of header.
        //
        // uri_ptr.end points to end of URI & HTTP version identifier.
        if (hi_util_in_bounds(start, end, uri_ptr.uri_end + 1))
            ptr = hi_client_extract_header(Session, ServerConf, &header_ptr, uri_ptr.uri_end+1, end, true_ip);

        if (header_ptr.header.uri)
        {
            Client->request.header_raw = header_ptr.header.uri;
            Client->request.header_raw_size = header_ptr.header.uri_end - header_ptr.header.uri;
            if(!Client->request.header_raw_size)
            {
                CLR_HEADER(Client);
            }
            else
            {
                hi_stats.req_headers++;
                Client->request.header_norm = header_ptr.header.uri;
                if (header_ptr.cookie.cookie)
                {
                    hi_stats.req_cookies++;
                    Client->request.cookie.cookie = header_ptr.cookie.cookie;
                    Client->request.cookie.cookie_end = header_ptr.cookie.cookie_end;
                    Client->request.cookie.next = header_ptr.cookie.next;
                    Client->request.cookie_norm = header_ptr.cookie.cookie;
                }
                else
                {
                    Client->request.cookie.cookie = NULL;
                    Client->request.cookie.cookie_end = NULL;
                    Client->request.cookie.next = NULL;
                    Client->request.cookie_norm = NULL;
                }
            }
        }
        else
        {
            CLR_HEADER(Client);
        }

        /* Need to skip over header and get to the body.
         * The unaptly named FindPipelineReq will do that. */
        ptr = FindPipelineReq(Session, uri_ptr.delimiter, end);
        //ptr = FindPipelineReq(Session, ptr, end);
        if(ptr)
        { 
            if (header_ptr.content_len.cont_len_start)
            {
                post_ptr.uri = ptr;
                post_ptr.uri_end = end;
                if((POST_END == hi_client_extract_post(
                    Session, ServerConf, ptr, end, &post_ptr, header_ptr.content_len.len )))
                {
                    hi_stats.post_params++;
                    Client->request.post_raw = post_ptr.uri;
                    Client->request.post_raw_size = post_ptr.uri_end - post_ptr.uri;
                    Client->request.post_norm = post_ptr.norm;
                    ptr = post_ptr.uri_end;
                }
                else
                {
                    CLR_POST(Client);
                }

                if ( ptr < end )
                    Client->request.pipeline_req = ptr;
            }
            else
            {
                uint8_t *tmp = memchr(ptr, (int)' ', end - ptr);

                if(tmp && ( (tmp - (uint8_t*)ptr) < 8 ))
                {
                    Client->request.pipeline_req = ptr;
                    CLR_POST(Client);
                }
                else
                {
                    post_ptr.uri = ptr;
                    post_ptr.uri_end = end;
     
                    if((POST_END == hi_client_extract_post(
                        Session, ServerConf, ptr, end, &post_ptr, -1)))
                    {
                        hi_stats.post_params++;
                        Client->request.post_raw = post_ptr.uri;
                        Client->request.post_raw_size = post_ptr.uri_end - post_ptr.uri;
                        Client->request.post_norm = post_ptr.norm;
                    }
                    else 
                    {
                        CLR_POST(Client);
                    }
                }
            }

            if(Client->request.post_raw && (ServerConf->post_depth > -1))
            {
                if(ServerConf->post_depth && ((int)Client->request.post_raw_size > ServerConf->post_depth))
                {
                    Client->request.post_raw_size = ServerConf->post_depth;
                }
            }
            else
            {
                CLR_POST(Client);
            }
        }
        else 
        {
            CLR_POST(Client);
            ptr = uri_ptr.delimiter;
        }
    }
    else 
    {
        CLR_HEADER(Client);
        CLR_POST(Client);
        if (method_ptr.uri)
        {
            Client->request.method_raw = method_ptr.uri;
            Client->request.method_size = method_ptr.uri_end - method_ptr.uri;
        }
        ptr = uri_ptr.delimiter;
    }

    /*
    **  If there is a pipelined request in this packet, we should always
    **  see the first space followed by text (which is the URI).  Without
    **  that first space, then we never get to the URI, so we should just
    **  return, since there is nothing else to inspect.
    */
    if(Client->request.pipeline_req)
    {
        if(uri_ptr.uri != uri_ptr.first_sp_end)
        {
            if(Session->server_conf->chunk_length)
                CheckChunkEncoding(Session, start, end, NULL, NULL, 0, 0, NULL, NULL);

            return HI_NONFATAL_ERR;
        }
    }
    else 
    {
           /*
            **  Find the next pipeline request, if one is there.  If we don't find
            **  a pipeline request, then we return NULL here, so this is always
            **  set to the correct value.
            */
        if(!ServerConf->no_pipeline)
        {
            if(post_ptr.uri)
            {
                Client->request.pipeline_req = 
                    FindPipelineReq(Session, post_ptr.delimiter, end);
            }
            else if(!Client->request.pipeline_req && uri_ptr.uri)
            {
                Client->request.pipeline_req = 
                    FindPipelineReq(Session, ptr, end);
            }
        }
        else
        {
            Client->request.pipeline_req = NULL;
        }
    }

    /*
    **  We set the HI_CLIENT variables from the URI_PTR structure.  We also
    **  do error checking for the values in this routine as well.
    */
    iRet = SetClientVars(Client, &uri_ptr, dsize);
    if (iRet)
    {
        return iRet;
    }

    /*
    **  One last check for an oversize directory.  This gets the long
    **  directory when there is a beginning slash and no other slashes
    **  until the end of the packet.
    **
    **  We do this check after we set the variables, just in case there
    **  was some errors while setting the variables.  This could save some
    **  false positives on a bad URI setting.
    */
    if(uri_ptr.uri_end)
        CheckLongDir(Session, &uri_ptr, uri_ptr.uri_end);

    /*
    **  Check for absolute URI and alert for proxy comm if necessary
    **
    **  NOTE:
    **    Also check ClientConf for proxy configuration so we don't
    **    alert on outbound requests from legitimate proxies.
    */
    if(uri_ptr.proxy && Session->global_conf->proxy_alert &&
       (!ServerConf->allow_proxy && !ClientConf->allow_proxy))
    {
        if(hi_eo_generate_event(Session, HI_EO_CLIENT_PROXY_USE))
        {
            hi_eo_client_event_log(Session, HI_EO_CLIENT_PROXY_USE, 
                    NULL, NULL);
        }
    }

    return HI_SUCCESS;
}

int hi_client_inspection(void *S, const unsigned char *data, int dsize, sfip_t **true_ip)
{
    HTTPINSPECT_GLOBAL_CONF *GlobalConf;
    HI_SESSION *Session;

    int iRet;

    if(!S || !data || dsize < 1)
    {
        return HI_INVALID_ARG;
    }

    Session = (HI_SESSION *)S;

    if(!Session->global_conf)
    {
        return HI_INVALID_ARG;
    }

    GlobalConf = Session->global_conf;

    /*
    **  We inspect the HTTP protocol in either stateful mode or
    **  stateless mode.
    */
    if(GlobalConf->inspection_type == HI_UI_CONFIG_STATEFUL)
    {
        /*
        **  This is where we do stateful inspection.
        */
        return HI_NONFATAL_ERR;
    }
    else
    {
        /*
        **  Otherwise we assume stateless inspection
        */
        iRet = StatelessInspection(Session, data, dsize, true_ip);
        if (iRet)
        {
            return iRet;
        }
    }

    return HI_SUCCESS;
}

/*
**  NAME
**    hi_client_init::
*/
/**
**  Initializes arrays and search algorithms depending on the type of
**  inspection that we are doing.
**  
**  @param GlobalConf pointer to the global configuration
**  
**  @return integer
**  
**  @retval HI_SUCCESS function successful.
*/
int hi_client_init(HTTPINSPECT_GLOBAL_CONF *GlobalConf)
{
    int iCtr;

    if(GlobalConf->inspection_type == HI_UI_CONFIG_STATEFUL)
    {
        /*
        **  We don't have to do anything here yet.
        */
    }
    else
    {
        memset(lookup_table, 0x00, sizeof(lookup_table));

        /*
        **  Set up the non-ASCII register for processing.
        */
        for(iCtr = 0x80; iCtr <= 0xff; iCtr++)
        {
            lookup_table[iCtr] = SetBinaryNorm;
        }
        lookup_table[0x00] = SetBinaryNorm;

        lookup_table[' ']  = NextNonWhiteSpace;
        lookup_table['\r'] = find_rfc_delimiter;
        lookup_table['\n'] = find_non_rfc_delimiter;

        /*
        **  ASCII encoding
        */
        lookup_table['%']  = SetPercentNorm;

        /*
        **  Looking for multiple slashes
        */
        lookup_table['/']  = SetSlashNorm;

        /*
        **  Looking for backslashs
        */
        lookup_table['\\'] = SetBackSlashNorm;

        lookup_table['+'] = SetPlusNorm;


        /*
        **  Look up parameter field, so we don't alert on long directory
        **  strings, when the next slash in the parameter field.
        */
        lookup_table['?'] = SetParamField;

        /*
        **  Look for absolute URI and proxy communication.
        */
        lookup_table[':'] = SetProxy;

    }

    return HI_SUCCESS;
}



/**
**  This was just an initial testing program for these functions.
*/
#ifdef TEST_ME

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
    HTTPINSPECT_GLOBAL_CONF GlobalConf;
    HI_SESSION *Session;
    HI_SI_INPUT SiInput;
    int iInspectMode = 0;
    int iRet;
    char data[] = "Hdslkfjaslfkj    HTTP/00000.111111";

    if((iRet = hi_ui_config_init_global_conf(&GlobalConf)))
    {
        printf("** error during global init.\n");
        return iRet;
    }

    if((iRet = hi_ui_config_default(&GlobalConf)))
    {
        printf("** error config default.\n");
        return iRet;
    }

    hi_ui_config_print_config(&GlobalConf);

    if((iRet = hi_client_init(&GlobalConf)))
    {
        printf("** error client init\n");
        return iRet;
    }
    
    SiInput.sip = inet_addr("1.1.1.1");
    SiInput.sip = inet_addr("1.1.1.2");
    SiInput.dport = 80;
    SiInput.sport = 7880;
    
    if((iRet = hi_si_session_inspection(&GlobalConf, &Session, &SiInput,
                    &iInspectMode)))
    {
        printf("** error session inspection\n");
        return iRet;
    }

    printf("** iInspectMode = %d\n", iInspectMode);
    if((iRet = hi_mi_mode_inspection(Session, iInspectMode, data, 
                    strlen(data))))
    {
        printf("** error mode_inspection\n");
        return iRet;
    }

    return 0;
}
#endif
    
    
