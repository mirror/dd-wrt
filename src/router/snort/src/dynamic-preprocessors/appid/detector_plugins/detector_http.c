/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2005-2013 Sourcefire, Inc.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "str_search.h"
#include "appInfoTable.h"
#include "common_util.h"
#include "detector_api.h"
#include "httpCommon.h"
#include "http_url_patterns.h"
#include "detector_http.h"
#include "fw_appid.h"
#include "client_app_base.h"

/* URL line patterns for identifying client */
#define HTTP_GET "GET "
#define HTTP_PUT "PUT "
#define HTTP_POST "POST "
#define HTTP_HEAD "HEAD "
#define HTTP_TRACE "TRACE "
#define HTTP_DELETE "DELETE "
#define HTTP_OPTIONS "OPTIONS "
#define HTTP_PROPFIND "PROPFIND "
#define HTTP_PROPPATCH "PROPPATCH "
#define HTTP_MKCOL "MKCOL "
#define HTTP_COPY "COPY "
#define HTTP_MOVE "MOVE "
#define HTTP_LOCK "LOCK "
#define HTTP_UNLOCK "UNLOCK "

#define HTTP_GET_SIZE (sizeof(HTTP_GET)-1)
#define HTTP_PUT_SIZE (sizeof(HTTP_PUT)-1)
#define HTTP_POST_SIZE (sizeof(HTTP_POST)-1)
#define HTTP_HEAD_SIZE (sizeof(HTTP_HEAD)-1)
#define HTTP_TRACE_SIZE (sizeof(HTTP_GET)-1)
#define HTTP_DELETE_SIZE (sizeof(HTTP_DELETE)-1)
#define HTTP_OPTIONS_SIZE (sizeof(HTTP_OPTIONS)-1)
#define HTTP_PROPFIND_SIZE (sizeof(HTTP_PROPFIND)-1)
#define HTTP_PROPPATCH_SIZE (sizeof(HTTP_PROPPATCH)-1)
#define HTTP_MKCOL_SIZE (sizeof(HTTP_GET)-1)
#define HTTP_COPY_SIZE (sizeof(HTTP_COPY)-1)
#define HTTP_MOVE_SIZE (sizeof(HTTP_MOVE)-1)
#define HTTP_LOCK_SIZE (sizeof(HTTP_LOCK)-1)
#define HTTP_UNLOCK_SIZE (sizeof(HTTP_UNLOCK)-1)

/* media type patterns*/
#define VIDEO_BANNER "video/"
#define AUDIO_BANNER "audio/"
#define APPLICATION_BANNER "application/"
#define QUICKTIME_BANNER "quicktime"
#define MPEG_BANNER "mpeg"
#define MPA_BANNER "mpa"
#define ROBUST_MPA_BANNER "robust-mpa"
#define MP4A_BANNER "mp4a-latm"
#define SHOCKWAVE_BANNER "x-shockwave-flash"
#define RSS_BANNER "rss+xml"
#define ATOM_BANNER "atom+xml"
#define MP4_BANNER "mp4"
#define WMV_BANNER "x-ms-wmv"
#define WMA_BANNER "x-ms-wma"
#define WAV_BANNER "wav"
#define X_WAV_BANNER "x-wav"
#define VND_WAV_BANNER "vnd.wav"
#define FLV_BANNER "x-flv"
#define M4V_BANNER "x-m4v"
#define GPP_BANNER "3gpp"
#define XSCPLS_BANNER "x-scpls"

#define VIDEO_BANNER_MAX_POS (sizeof(VIDEO_BANNER)-2)
#define AUDIO_BANNER_MAX_POS (sizeof(AUDIO_BANNER)-2)
#define APPLICATION_BANNER_MAX_POS (sizeof(APPLICATION_BANNER)-2)
#define QUICKTIME_BANNER_MAX_POS (sizeof(QUICKTIME_BANNER)-2)
#define MPEG_BANNER_MAX_POS (sizeof(MPEG_BANNER)-2)
#define MPA_BANNER_MAX_POS (sizeof(MPA_BANNER)-2)
#define ROBUST_MPA_BANNER_MAX_POS (sizeof(ROBUST_MPA_BANNER)-2)
#define MP4A_BANNER_MAX_POS (sizeof(MP4A_BANNER)-2)
#define SHOCKWAVE_BANNER_MAX_POS (sizeof(SHOCKWAVE_BANNER)-2)
#define RSS_BANNER_MAX_POS (sizeof(RSS_BANNER)-2)
#define ATOM_BANNER_MAX_POS (sizeof(ATOM_BANNER)-2)
#define MP4_BANNER_MAX_POS (sizeof(MP4_BANNER)-2)
#define WMV_BANNER_MAX_POS (sizeof(WMV_BANNER)-2)
#define WMA_BANNER_MAX_POS (sizeof(WMA_BANNER)-2)
#define WAV_BANNER_MAX_POS (sizeof(WAV_BANNER)-2)
#define X_WAV_BANNER_MAX_POS (sizeof(X_WAV_BANNER)-2)
#define VND_WAV_BANNER_MAX_POS (sizeof(VND_WAV_BANNER)-2)
#define FLV_BANNER_MAX_POS (sizeof(FLV_BANNER)-2)
#define M4V_BANNER_MAX_POS (sizeof(M4V_BANNER)-2)
#define GPP_BANNER_MAX_POS (sizeof(GPP_BANNER)-2)
#define XSCPLS_BANNER_MAX_POS (sizeof(XSCPLS_BANNER)-2)

/* version patterns*/
static const char MSIE_PATTERN[] = "MSIE";
static const char KONQUEROR_PATTERN[] = "Konqueror";
static const char SKYPE_PATTERN[] = "Skype";
static const char BITTORRENT_PATTERN[] = "BitTorrent";
static const char FIREFOX_PATTERN[] = "Firefox";
static const char WGET_PATTERN[] = "Wget/";
static const char CURL_PATTERN[] = "curl";
static const char GOOGLE_DESKTOP_PATTERN[] = "Google Desktop";
static const char PICASA_PATTERN[] = "Picasa";
static const char SAFARI_PATTERN[] = "Safari";
static const char CHROME_PATTERN[] = "Chrome";
static const char MOBILE_PATTERN[] = "Mobile";
static const char BLACKBERRY_PATTERN[] = "BlackBerry";
static const char ANDROID_PATTERN[] = "Android";
static const char MEDIAPLAYER_PATTERN[] = "Windows-Media-Player";
static const char APPLE_EMAIL_PATTERN[] = "Maci";
static const char *APPLE_EMAIL_PATTERNS[] = { "Mozilla/5.0","AppleWebKit","(KHTML, like Gecko)"};

/* "fake" patterns for user-agent matching */
static const char VERSION_PATTERN[] = "Version";
#define VERSION_PATTERN_SIZE (sizeof(VERSION_PATTERN)-1)
#define FAKE_VERSION_APP_ID 3

/* proxy patterns*/
static const char SQUID_PATTERN[] = "squid";
#define SQUID_PATTERN_SIZE (sizeof(SQUID_PATTERN)-1)

static const char MYSPACE_PATTERN[] = "myspace.com";
static const char GMAIL_PATTERN[] = "gmail.com";
static const char GMAIL_PATTERN2[] = "mail.google.com";
static const char AOL_PATTERN[] = "webmail.aol.com";
static const char MSUP_PATTERN[] = "update.microsoft.com";
static const char MSUP_PATTERN2[] = "windowsupdate.com";
static const char YAHOO_MAIL_PATTERN[] = "mail.yahoo.com";
static const char YAHOO_TB_PATTERN[] = "rd.companion.yahoo.com";
static const char ADOBE_UP_PATTERN[] = "swupmf.adobe.com";
static const char HOTMAIL_PATTERN1[] = "hotmail.com";
static const char HOTMAIL_PATTERN2[] = "mail.live.com";
static const char GOOGLE_TB_PATTERN[] = "toolbarqueries.google.com";
#define MYSPACE_PATTERN_SIZE (sizeof(MYSPACE_PATTERN)-1)
#define GMAIL_PATTERN_SIZE (sizeof(GMAIL_PATTERN)-1)
#define GMAIL_PATTERN2_SIZE (sizeof(GMAIL_PATTERN2)-1)
#define AOL_PATTERN_SIZE (sizeof(AOL_PATTERN)-1)
#define MSUP_PATTERN_SIZE (sizeof(MSUP_PATTERN)-1)
#define MSUP_PATTERN2_SIZE (sizeof(MSUP_PATTERN2)-1)
#define YAHOO_MAIL_PATTERN_SIZE (sizeof(YAHOO_MAIL_PATTERN)-1)
#define YAHOO_TB_PATTERN_SIZE (sizeof(YAHOO_TB_PATTERN)-1)
#define ADOBE_UP_PATTERN_SIZE (sizeof(ADOBE_UP_PATTERN)-1)
#define HOTMAIL_PATTERN1_SIZE (sizeof(HOTMAIL_PATTERN1)-1)
#define HOTMAIL_PATTERN2_SIZE (sizeof(HOTMAIL_PATTERN2)-1)
#define GOOGLE_TB_PATTERN_SIZE (sizeof(GOOGLE_TB_PATTERN)-1)

#define COMPATIBLE_BROWSER_STRING " (Compat)"

typedef struct _MatchedPatterns {
    DetectorHTTPPattern *mpattern;
    int index;
    struct _MatchedPatterns *next;
} MatchedPatterns;

static DetectorHTTPPattern content_type_patterns[] =
{
    { SINGLE, 0, APP_ID_QUICKTIME, 0,
        sizeof(QUICKTIME_BANNER)-1, (u_int8_t *) QUICKTIME_BANNER, APP_ID_QUICKTIME
    },
    { SINGLE, 0, APP_ID_MPEG, 0,
        sizeof(MPEG_BANNER)-1, (u_int8_t *) MPEG_BANNER, APP_ID_MPEG
    },
    { SINGLE, 0, APP_ID_MPEG, 0,
        sizeof(MPA_BANNER)-1, (u_int8_t *) MPA_BANNER, APP_ID_MPEG
    },
    { SINGLE, 0, APP_ID_MPEG, 0,
        sizeof(MP4A_BANNER)-1, (u_int8_t *) MP4A_BANNER, APP_ID_MPEG
    },
    { SINGLE, 0, APP_ID_MPEG, 0,
        sizeof(ROBUST_MPA_BANNER)-1, (u_int8_t *) ROBUST_MPA_BANNER, APP_ID_MPEG
    },
    { SINGLE, 0, APP_ID_MPEG, 0,
        sizeof(XSCPLS_BANNER)-1, (u_int8_t *) XSCPLS_BANNER, APP_ID_MPEG
    },
    { SINGLE, 0, APP_ID_SHOCKWAVE, 0,
        sizeof(SHOCKWAVE_BANNER)-1, (u_int8_t *) SHOCKWAVE_BANNER, APP_ID_SHOCKWAVE
    },
    { SINGLE, 0, APP_ID_RSS, 0,
        sizeof(RSS_BANNER)-1, (u_int8_t *) RSS_BANNER, APP_ID_RSS
    },
    { SINGLE, 0, APP_ID_ATOM, 0,
        sizeof(ATOM_BANNER)-1, (u_int8_t *) ATOM_BANNER, APP_ID_ATOM
    },
    { SINGLE, 0, APP_ID_MP4, 0,
        sizeof(MP4_BANNER)-1, (u_int8_t *) MP4_BANNER, APP_ID_MP4
    },
    { SINGLE, 0, APP_ID_WMV, 0,
        sizeof(WMV_BANNER)-1, (u_int8_t *) WMV_BANNER, APP_ID_WMV
    },
    { SINGLE, 0, APP_ID_WMA, 0,
        sizeof(WMA_BANNER)-1, (u_int8_t *) WMA_BANNER, APP_ID_WMA
    },
    { SINGLE, 0, APP_ID_WAV, 0,
        sizeof(WAV_BANNER)-1, (u_int8_t *) WAV_BANNER, APP_ID_WAV
    },
    { SINGLE, 0, APP_ID_WAV, 0,
        sizeof(X_WAV_BANNER)-1, (u_int8_t *) X_WAV_BANNER, APP_ID_WAV
    },
    { SINGLE, 0, APP_ID_WAV, 0,
        sizeof(VND_WAV_BANNER)-1, (u_int8_t *) VND_WAV_BANNER, APP_ID_WAV
    },
    { SINGLE, 0, APP_ID_FLASH_VIDEO, 0,
        sizeof(FLV_BANNER)-1, (u_int8_t *) FLV_BANNER, APP_ID_FLASH_VIDEO
    },
    { SINGLE, 0, APP_ID_FLASH_VIDEO, 0,
        sizeof(M4V_BANNER)-1, (u_int8_t *) M4V_BANNER, APP_ID_FLASH_VIDEO
    },
    { SINGLE, 0, APP_ID_FLASH_VIDEO, 0,
        sizeof(GPP_BANNER)-1, (u_int8_t *) GPP_BANNER, APP_ID_FLASH_VIDEO
    },
    { SINGLE, 0, APP_ID_GENERIC, 0,
        sizeof(VIDEO_BANNER)-1, (u_int8_t *) VIDEO_BANNER, APP_ID_GENERIC
    },
    { SINGLE, 0, APP_ID_GENERIC, 0,
        sizeof(AUDIO_BANNER)-1, (u_int8_t *) AUDIO_BANNER, APP_ID_GENERIC
    },
};

static DetectorHTTPPattern via_http_detector_patterns[] =
{
    { SINGLE, APP_ID_SQUID, 0, 0,
        SQUID_PATTERN_SIZE, (u_int8_t *) SQUID_PATTERN, APP_ID_SQUID
    },
};

static DetectorHTTPPattern host_payload_http_detector_patterns[] =
{
    { SINGLE, 0, 0, APP_ID_MYSPACE,
      MYSPACE_PATTERN_SIZE, (u_int8_t *) MYSPACE_PATTERN, APP_ID_MYSPACE
    },
    { SINGLE, 0, 0, APP_ID_GMAIL,
      GMAIL_PATTERN_SIZE, (u_int8_t *) GMAIL_PATTERN, APP_ID_GMAIL,
    },
    { SINGLE, 0, 0, APP_ID_GMAIL,
      GMAIL_PATTERN2_SIZE, (u_int8_t *) GMAIL_PATTERN2, APP_ID_GMAIL,
    },
    { SINGLE, 0, 0, APP_ID_AOL_EMAIL,
      AOL_PATTERN_SIZE, (u_int8_t *) AOL_PATTERN, APP_ID_AOL_EMAIL,
    },
    { SINGLE, 0, 0, APP_ID_MICROSOFT_UPDATE,
      MSUP_PATTERN_SIZE, (u_int8_t *) MSUP_PATTERN, APP_ID_MICROSOFT_UPDATE,
    },
    { SINGLE, 0, 0, APP_ID_MICROSOFT_UPDATE,
      MSUP_PATTERN2_SIZE, (u_int8_t *) MSUP_PATTERN2, APP_ID_MICROSOFT_UPDATE,
    },
    { SINGLE, 0, 0, APP_ID_YAHOOMAIL,
      YAHOO_MAIL_PATTERN_SIZE, (u_int8_t *)YAHOO_MAIL_PATTERN, APP_ID_YAHOOMAIL,
    },
    { SINGLE, 0, 0, APP_ID_YAHOO_TOOLBAR,
      YAHOO_TB_PATTERN_SIZE, (u_int8_t *)YAHOO_TB_PATTERN, APP_ID_YAHOO_TOOLBAR,
    },
    { SINGLE, 0, 0, APP_ID_ADOBE_UPDATE,
      ADOBE_UP_PATTERN_SIZE, (u_int8_t *)ADOBE_UP_PATTERN, APP_ID_ADOBE_UPDATE,
    },
    { SINGLE, 0, 0, APP_ID_HOTMAIL,
      HOTMAIL_PATTERN1_SIZE, (u_int8_t *)HOTMAIL_PATTERN1, APP_ID_HOTMAIL,
    },
    { SINGLE, 0, 0, APP_ID_HOTMAIL,
      HOTMAIL_PATTERN2_SIZE, (u_int8_t *)HOTMAIL_PATTERN2, APP_ID_HOTMAIL,
    },
    { SINGLE, 0, 0, APP_ID_GOOGLE_TOOLBAR,
      GOOGLE_TB_PATTERN_SIZE, (u_int8_t *)GOOGLE_TB_PATTERN, APP_ID_GOOGLE_TOOLBAR,
    },
};

static DetectorHTTPPattern client_agent_patterns[] =
{
    { USER_AGENT_HEADER, 0, FAKE_VERSION_APP_ID, 0,
      VERSION_PATTERN_SIZE, (u_int8_t *)VERSION_PATTERN, FAKE_VERSION_APP_ID,
    },
    { USER_AGENT_HEADER, APP_ID_HTTP, APP_ID_INTERNET_EXPLORER, 0,
      sizeof(MSIE_PATTERN)-1, (u_int8_t *)MSIE_PATTERN, APP_ID_INTERNET_EXPLORER,
    },
    { USER_AGENT_HEADER, APP_ID_HTTP, APP_ID_KONQUEROR, 0,
      sizeof(KONQUEROR_PATTERN)-1, (u_int8_t *)KONQUEROR_PATTERN, APP_ID_KONQUEROR,
    },
    { USER_AGENT_HEADER, APP_ID_SKYPE_AUTH, APP_ID_SKYPE, 0,
      sizeof(SKYPE_PATTERN)-1, (u_int8_t *)SKYPE_PATTERN, APP_ID_SKYPE,
    },
    { USER_AGENT_HEADER, APP_ID_BITTORRENT, APP_ID_BITTORRENT, 0,
      sizeof(BITTORRENT_PATTERN)-1, (u_int8_t *)BITTORRENT_PATTERN, APP_ID_BITTORRENT,
    },
    { USER_AGENT_HEADER, APP_ID_HTTP, APP_ID_FIREFOX, 0,
      sizeof(FIREFOX_PATTERN)-1, (u_int8_t *)FIREFOX_PATTERN, APP_ID_FIREFOX,
    },
    { USER_AGENT_HEADER, APP_ID_HTTP, APP_ID_WGET, 0,
      sizeof(WGET_PATTERN)-1, (u_int8_t *)WGET_PATTERN, APP_ID_WGET,
    },
    { USER_AGENT_HEADER, APP_ID_HTTP, APP_ID_CURL, 0,
      sizeof(CURL_PATTERN)-1, (u_int8_t *)CURL_PATTERN, APP_ID_CURL,
    },
    { USER_AGENT_HEADER, APP_ID_HTTP, APP_ID_GOOGLE_DESKTOP, 0,
      sizeof(GOOGLE_DESKTOP_PATTERN)-1, (u_int8_t *)GOOGLE_DESKTOP_PATTERN, APP_ID_GOOGLE_DESKTOP,
    },
    { USER_AGENT_HEADER, APP_ID_HTTP, APP_ID_PICASA, 0,
      sizeof(PICASA_PATTERN)-1, (u_int8_t *)PICASA_PATTERN, APP_ID_PICASA,
    },
    { USER_AGENT_HEADER, APP_ID_HTTP, APP_ID_SAFARI, 0,
      sizeof(SAFARI_PATTERN)-1, (u_int8_t *)SAFARI_PATTERN, APP_ID_SAFARI,
    },
    { USER_AGENT_HEADER, APP_ID_HTTP, APP_ID_CHROME, 0,
      sizeof(CHROME_PATTERN)-1, (u_int8_t *)CHROME_PATTERN, APP_ID_CHROME,
    },
    { USER_AGENT_HEADER, APP_ID_HTTP, APP_ID_SAFARI_MOBILE_DUMMY, 0,
      sizeof(MOBILE_PATTERN)-1, (u_int8_t *)MOBILE_PATTERN, APP_ID_SAFARI_MOBILE_DUMMY,
    },
    { USER_AGENT_HEADER, APP_ID_HTTP, APP_ID_BLACKBERRY_BROWSER, 0,
      sizeof(BLACKBERRY_PATTERN)-1, (u_int8_t *)BLACKBERRY_PATTERN, APP_ID_BLACKBERRY_BROWSER,
    },
    { USER_AGENT_HEADER, APP_ID_HTTP, APP_ID_ANDROID_BROWSER, 0,
      sizeof(ANDROID_PATTERN)-1, (u_int8_t *)ANDROID_PATTERN, APP_ID_ANDROID_BROWSER,
    },
    { USER_AGENT_HEADER, APP_ID_HTTP, APP_ID_WINDOWS_MEDIA_PLAYER, 0,
      sizeof(MEDIAPLAYER_PATTERN)-1, (u_int8_t *)MEDIAPLAYER_PATTERN, APP_ID_WINDOWS_MEDIA_PLAYER,
    },
    { USER_AGENT_HEADER, APP_ID_HTTP, APP_ID_APPLE_EMAIL, 0,
          sizeof(APPLE_EMAIL_PATTERN)-1, (u_int8_t *)APPLE_EMAIL_PATTERN, APP_ID_APPLE_EMAIL,
    },
};

struct _PATTERN_HTTP_HEADER;
typedef struct _HEADER_PATTERN
{
    int id;
    uint8_t *data;
    unsigned length;
} HeaderPattern;

static const char HTTP_HEADER_CONTENT_TYPE[] = "Content-Type: ";
static const char HTTP_HEADER_SERVER_FIELD[] = "Server: ";
static const char HTTP_HEADER_X_WORKING_WITH[] = "X-Working-With: ";
static const char HTTP_HEADER_CRLF[] = "\r";
static const char HTTP_HEADER_LF[] = "\n";

#define HTTP_HEADER_CONTENT_TYPE_SIZE (sizeof(HTTP_HEADER_CONTENT_TYPE)-1)
#define HTTP_HEADER_SERVER_FIELD_SIZE (sizeof(HTTP_HEADER_SERVER_FIELD)-1)
#define HTTP_HEADER_X_WORKING_WITH_SIZE (sizeof(HTTP_HEADER_X_WORKING_WITH)-1)
#define HTTP_HEADER_CRLF_SIZE (sizeof(HTTP_HEADER_CRLF)-1)
#define HTTP_HEADER_LF_SIZE (sizeof(HTTP_HEADER_LF)-1)

static HeaderPattern header_patterns[] =
{
    {HTTP_ID_CONTENT_TYPE, (uint8_t *) HTTP_HEADER_CONTENT_TYPE,HTTP_HEADER_CONTENT_TYPE_SIZE},
    {HTTP_ID_SERVER, (uint8_t *) HTTP_HEADER_SERVER_FIELD,HTTP_HEADER_SERVER_FIELD_SIZE},
    {HTTP_ID_COPY, (uint8_t *) HTTP_COPY, HTTP_COPY_SIZE},
    {HTTP_ID_DELETE, (uint8_t *) HTTP_DELETE, HTTP_DELETE_SIZE},
    {HTTP_ID_GET, (uint8_t *) HTTP_GET, HTTP_GET_SIZE},
    {HTTP_ID_HEAD, (uint8_t *) HTTP_HEAD, HTTP_HEAD_SIZE},
    {HTTP_ID_OPTIONS, (uint8_t *) HTTP_OPTIONS, HTTP_OPTIONS_SIZE},
    {HTTP_ID_PROPFIND, (uint8_t *) HTTP_PROPFIND, HTTP_PROPFIND_SIZE},
    {HTTP_ID_PROPPATCH, (uint8_t *) HTTP_PROPPATCH, HTTP_PROPPATCH_SIZE},
    {HTTP_ID_MKCOL, (uint8_t *) HTTP_MKCOL, HTTP_MKCOL_SIZE},
    {HTTP_ID_LOCK, (uint8_t *) HTTP_LOCK, HTTP_LOCK_SIZE},
    {HTTP_ID_MOVE, (uint8_t *) HTTP_MOVE, HTTP_MOVE_SIZE},
    {HTTP_ID_PUT, (uint8_t *) HTTP_PUT, HTTP_PUT_SIZE},
    {HTTP_ID_TRACE, (uint8_t *) HTTP_TRACE, HTTP_TRACE_SIZE},
    {HTTP_ID_UNLOCK, (uint8_t *) HTTP_UNLOCK, HTTP_UNLOCK_SIZE},
    {HTTP_ID_X_WORKING_WITH, (uint8_t *) HTTP_HEADER_X_WORKING_WITH,HTTP_HEADER_X_WORKING_WITH_SIZE},
    {HTTP_ID_LEN, (uint8_t *) HTTP_HEADER_CRLF, HTTP_HEADER_CRLF_SIZE},
    {HTTP_ID_LEN, (uint8_t *) HTTP_HEADER_LF, HTTP_HEADER_LF_SIZE}
};

static int content_pattern_match(void* id, void *unused_tree, int index, void* data, void *unused_neg)
{
    MatchedPatterns *cm;
    MatchedPatterns **matches = (MatchedPatterns **)data;
    DetectorHTTPPattern *target = (DetectorHTTPPattern *)id;

    if (!(cm = (MatchedPatterns *)malloc(sizeof(MatchedPatterns))))
        return 1;

    cm->mpattern = target;
    cm->index = index;
    cm->next = *matches;
    *matches = cm;

    return 0;
}

static int chp_pattern_match(void *id, void *unused_tree, int index, void *data, void *unused_neg)
{
    MatchedCHPAction *new_match;
    MatchedCHPAction *current_search;
    MatchedCHPAction *prev_search;
    MatchedCHPAction **matches = (MatchedCHPAction **)data;
    CHPAction *target = (CHPAction *)id;

    if (!(new_match = (MatchedCHPAction *)malloc(sizeof(MatchedCHPAction))))
        return 1;

    new_match->mpattern = target;
    new_match->index = index;

    // preserving order is required: sort by appIdInstance, then by precedence
    for (current_search = *matches, prev_search = NULL;
          NULL != current_search;
          prev_search = current_search, current_search = current_search->next)
    {
        register CHPAction *match_data = current_search->mpattern;
        if (target->appIdInstance < match_data->appIdInstance)
            break;
        if (target->appIdInstance == match_data->appIdInstance)
        {
            if (target->precedence < match_data->precedence)
                break;
        }
    }


    if (prev_search)
    {
        new_match->next = prev_search->next;
        prev_search->next = new_match;
    }
    else
    {
        // insert at head of list.
        new_match->next = *matches;
        *matches = new_match;
    }
    return 0;
}

#define CHP_TALLY_GROWTH_FACTOR  10
static inline void chp_add_candidate_to_tally( CHPMatchTally **ppTally, CHPApp *chpapp )
{
    int index;
    CHPMatchTally *pTally = *ppTally;
    if (!pTally)
    {
        pTally = (CHPMatchTally *)malloc(sizeof(CHPMatchTally)+CHP_TALLY_GROWTH_FACTOR*sizeof(CHPMatchCandidate));
        if (!pTally)
            return;
        pTally->in_use_elements = 0;
        pTally->allocated_elements = CHP_TALLY_GROWTH_FACTOR;
        *ppTally = pTally;
    }
    for (index=0; index < pTally->in_use_elements; index++ )
    {
        if (chpapp == pTally->item[index].chpapp)
        {
            pTally->item[index].key_pattern_countdown--;
            return;
        }
    }
    // Not found. Add to array
    if (pTally->in_use_elements == pTally->allocated_elements)
    {
        int newCount = pTally->allocated_elements + CHP_TALLY_GROWTH_FACTOR;
        CHPMatchTally *pNewTally = (CHPMatchTally *)realloc( pTally, sizeof(CHPMatchTally)+newCount*sizeof(CHPMatchCandidate));
        if (pNewTally)
        {
            *ppTally = pTally = pNewTally;
            pTally->allocated_elements = newCount;
        }
        else
            return; // failed to allocate a bigger chunk
    }
    // index == pTally->in_use_elements
    pTally->in_use_elements++;
    pTally->item[index].chpapp = chpapp;
    pTally->item[index].key_pattern_length_sum = chpapp->key_pattern_length_sum;
    pTally->item[index].key_pattern_countdown = chpapp->key_pattern_count - 1; // the count would have included this find.
}

typedef struct _CHPTallyAndActions {
    CHPMatchTally *pTally;
    MatchedCHPAction *matches;
} CHPTallyAndActions;

// In addition to creating the linked list of matching actions this function will
// create the CHPMatchTally needed to find the longest matching pattern.
static int chp_key_pattern_match(void *id, void *unused_tree, int index, void *data, void *unused_neg)
{
    CHPTallyAndActions *pTallyAndActions = (CHPTallyAndActions*)data;
    CHPAction *target = (CHPAction *)id;

    if (target->key_pattern)
    {
        // We have a match from a key pattern. We need to have it's parent chpapp represented in the tally.
        // If the chpapp has never been seen then add an item to the tally's array
        // else decrement the count of expected key_patterns until zero so that we know when we have them all.
        chp_add_candidate_to_tally( &pTallyAndActions->pTally, target->chpapp );
    }
    return chp_pattern_match(id, unused_tree, index, &pTallyAndActions->matches, unused_neg);
}


static int http_pattern_match(void* id, void *unused_tree, int index, void* data, void *unused_neg)
{
    MatchedPatterns *cm = NULL;
    MatchedPatterns **tmp;
    MatchedPatterns **matches = (MatchedPatterns **)data;
    DetectorHTTPPattern *target = (DetectorHTTPPattern *)id;

    /* make sure we haven't already seen this pattern */
    for (tmp = matches;
         *tmp;
         tmp = &(*tmp)->next)
    {
        cm = *tmp;
    }

    if (!*tmp)
    {
        if (!(cm = (MatchedPatterns *)malloc(sizeof(MatchedPatterns))))
            return 1;
        else
        {
            cm->mpattern = target;
            cm->index = index;
            cm->next = NULL;
            *tmp = cm;
        }
    }

    /* if its one of the host patterns, return after first match*/
    if (cm->mpattern->seq == SINGLE) return 1;
    else return 0;
}

static void* processPatterns(
        DetectorHTTPPattern *patternList,
        size_t patternListCount,
        size_t *patternIndex,
        HTTPListElement* luaPatternList
        )
{
    void *patternMatcher;
    HTTPListElement* element;
    u_int32_t i;

    if (!(patternMatcher = _dpd.searchAPI->search_instance_new_ex(MPSE_ACF)))
        return NULL;

    for (i=0; i < patternListCount; i++)
    {
        _dpd.searchAPI->search_instance_add_ex(patternMatcher,
                (char *)patternList[i].pattern,
                patternList[i].pattern_size,
                &patternList[i],
                STR_SEARCH_CASE_SENSITIVE);
    }

    /* Add patterns from Lua API */
    for(element = luaPatternList; element != 0; element = element->next)
    {
        _dpd.searchAPI->search_instance_add_ex(patternMatcher,
                (char *)element->detectorHTTPPattern.pattern,
                element->detectorHTTPPattern.pattern_size,
                &element->detectorHTTPPattern,
                STR_SEARCH_CASE_SENSITIVE);
    }

    _dpd.searchAPI->search_instance_prep(patternMatcher);

    return patternMatcher;
}

static int processHostPatterns(
        DetectorHTTPPattern *patternList,
        size_t patternListCount,
        HTTPListElement* luaPatternList,
        DetectorAppUrlList *urlPatternList,
        DetectorAppUrlList *RTMPUrlList,
        tDetectorHttpConfig *pHttpConfig
        )
{
    HTTPListElement* element;
    u_int32_t i;
    DetectorAppUrlPattern *appUrlPattern;

    if (!pHttpConfig->hostUrlMatcher)
        pHttpConfig->hostUrlMatcher = mlmpCreate();

    if (!pHttpConfig->RTMPHostUrlMatcher)
        pHttpConfig->RTMPHostUrlMatcher = mlmpCreate();

    for (i=0; i < patternListCount; i++)
    {
        if (addMlmpPattern(pHttpConfig->hostUrlMatcher,  &pHttpConfig->hostUrlPatternsList,
                patternList[i].pattern, patternList[i].pattern_size,
                NULL, 0, NULL, 0, patternList[i].appId, patternList[i].payload, patternList[i].service_id,
                patternList[i].client_app, patternList[i].seq) < 0)
            return -1;
    }

    for(element = luaPatternList; element != 0; element = element->next)
    {
        if (addMlmpPattern(pHttpConfig->hostUrlMatcher, &pHttpConfig->hostUrlPatternsList,
                element->detectorHTTPPattern.pattern, element->detectorHTTPPattern.pattern_size,
                NULL, 0, NULL, 0, element->detectorHTTPPattern.appId, element->detectorHTTPPattern.payload, element->detectorHTTPPattern.service_id,
                element->detectorHTTPPattern.client_app, element->detectorHTTPPattern.seq) < 0)
            return -1;
    }

    for (i = 0; i < RTMPUrlList->usedCount; i++)
    {
        appUrlPattern = RTMPUrlList->urlPattern[i];
        if (addMlmpPattern(pHttpConfig->RTMPHostUrlMatcher, &pHttpConfig->hostUrlPatternsList,
                appUrlPattern->patterns.host.pattern, appUrlPattern->patterns.host.patternSize,
                appUrlPattern->patterns.path.pattern, appUrlPattern->patterns.path.patternSize,
                appUrlPattern->userData.query.pattern, appUrlPattern->userData.query.patternSize,
                appUrlPattern->userData.appId,         appUrlPattern->userData.payload,
                appUrlPattern->userData.service_id,    appUrlPattern->userData.client_app,
                SINGLE) < 0)
            return -1;
    }

    for (i = 0; i < urlPatternList->usedCount; i++)
    {
        appUrlPattern = urlPatternList->urlPattern[i];
        if (addMlmpPattern(pHttpConfig->hostUrlMatcher, &pHttpConfig->hostUrlPatternsList,
                appUrlPattern->patterns.host.pattern, appUrlPattern->patterns.host.patternSize,
                appUrlPattern->patterns.path.pattern, appUrlPattern->patterns.path.patternSize,
                appUrlPattern->userData.query.pattern, appUrlPattern->userData.query.patternSize,
                appUrlPattern->userData.appId,         appUrlPattern->userData.payload,
                appUrlPattern->userData.service_id,    appUrlPattern->userData.client_app,
                SINGLE) < 0)
            return -1;
    }

    mlmpProcessPatterns(pHttpConfig->hostUrlMatcher);
    mlmpProcessPatterns(pHttpConfig->RTMPHostUrlMatcher);
    return 0;
}

static void* processContentTypePatterns(
        DetectorHTTPPattern *patternList,
        size_t patternListCount,
        HTTPListElement* luaPatternList,
        size_t *patternIndex)
{
    void *patternMatcher;
    HTTPListElement* element;
    u_int32_t i;

    if (!(patternMatcher = _dpd.searchAPI->search_instance_new_ex(MPSE_ACF)))
        return NULL;

    for (i=0; i < patternListCount; i++)
    {
        _dpd.searchAPI->search_instance_add_ex(patternMatcher,
                (char  *)patternList[i].pattern,
                patternList[i].pattern_size,
                &patternList[i],
                STR_SEARCH_CASE_SENSITIVE);
    }

    /* Add patterns from Lua API */
    for(element = luaPatternList; element; element = element->next)
    {
        _dpd.searchAPI->search_instance_add_ex(patternMatcher,
                (char  *)element->detectorHTTPPattern.pattern,
                element->detectorHTTPPattern.pattern_size,
                &element->detectorHTTPPattern,
                STR_SEARCH_CASE_SENSITIVE);
    }

    _dpd.searchAPI->search_instance_prep(patternMatcher);

    return patternMatcher;
}

static int processCHPList(CHPListElement *chplist, tDetectorHttpConfig *pHttpConfig)
{
    CHPListElement *chpe;
    int i;

    for (i = 0; i < sizeof(pHttpConfig->chp_matchers)/sizeof(pHttpConfig->chp_matchers[0]); i++)
    {
        if (!(pHttpConfig->chp_matchers[i] = _dpd.searchAPI->search_instance_new_ex(MPSE_ACF)))
            return 0;
    }

    for(chpe = chplist; chpe; chpe=chpe->next)
    {
        _dpd.searchAPI->search_instance_add_ex(pHttpConfig->chp_matchers[chpe->chp_action.ptype],
                           (char *)chpe->chp_action.pattern,
                           chpe->chp_action.psize,
                           &chpe->chp_action,
                           STR_SEARCH_CASE_INSENSITIVE);
    }

    for (i = 0; i < sizeof(pHttpConfig->chp_matchers)/sizeof(pHttpConfig->chp_matchers[0]); i++)
    {
        _dpd.searchAPI->search_instance_prep(pHttpConfig->chp_matchers[i]);
    }

    return 1;
}
static void* registerHeaderPatterns(
        HeaderPattern *patternList,
        size_t patternListCount)
{
    void *patternMatcher;
    u_int32_t i;

    if (!(patternMatcher = _dpd.searchAPI->search_instance_new_ex(MPSE_ACF)))
        return NULL;

    for (i=0; i < patternListCount; i++)
    {
        /* add patterns with case insensitivity */
        _dpd.searchAPI->search_instance_add_ex(patternMatcher,
                (char  *)patternList[i].data,
                patternList[i].length,
                &patternList[i],
                STR_SEARCH_CASE_INSENSITIVE);
    }

    _dpd.searchAPI->search_instance_prep(patternMatcher);

    return patternMatcher;
}
#define HTTP_FIELD_PREFIX_USER_AGENT    "\r\nUser-Agent: "
#define HTTP_FIELD_PREFIX_USER_AGENT_SIZE (sizeof(HTTP_FIELD_PREFIX_USER_AGENT)-1)
#define HTTP_FIELD_PREFIX_HOST    "\r\nHost: "
#define HTTP_FIELD_PREFIX_HOST_SIZE (sizeof(HTTP_FIELD_PREFIX_HOST)-1)
#define HTTP_FIELD_PREFIX_REFERER    "\r\nReferer: "
#define HTTP_FIELD_PREFIX_REFERER_SIZE (sizeof(HTTP_FIELD_PREFIX_REFERER)-1)
#define HTTP_FIELD_PREFIX_URI    " "
#define HTTP_FIELD_PREFIX_URI_SIZE (sizeof(HTTP_FIELD_PREFIX_URI)-1)
#define HTTP_FIELD_PREFIX_COOKIE    "\r\nCookie: "
#define HTTP_FIELD_PREFIX_COOKIE_SIZE (sizeof(HTTP_FIELD_PREFIX_COOKIE)-1)

typedef struct _FIELD_PATTERN
{
    PatternType patternType;
    uint8_t *data;
    unsigned length;
} FieldPattern;

static FieldPattern http_field_patterns[] =
{
    {URI_PT, (uint8_t *) HTTP_FIELD_PREFIX_URI, HTTP_FIELD_PREFIX_URI_SIZE},
    {HOST_PT, (uint8_t *) HTTP_FIELD_PREFIX_HOST,HTTP_FIELD_PREFIX_HOST_SIZE},
    {REFERER_PT, (uint8_t *) HTTP_FIELD_PREFIX_REFERER, HTTP_FIELD_PREFIX_REFERER_SIZE},
    {COOKIE_PT, (uint8_t *) HTTP_FIELD_PREFIX_COOKIE, HTTP_FIELD_PREFIX_COOKIE_SIZE},
    {AGENT_PT, (uint8_t *) HTTP_FIELD_PREFIX_USER_AGENT,HTTP_FIELD_PREFIX_USER_AGENT_SIZE},
};

static void* processHttpFieldPatterns(
        FieldPattern* patternList,
        size_t patternListCount)
{
    void *patternMatcher;
    u_int32_t i;

    if (!(patternMatcher = _dpd.searchAPI->search_instance_new_ex(MPSE_ACF)))
        return NULL;

    for (i=0; i < patternListCount; i++)
    {
        /* add patterns with case sensitivity */
        _dpd.searchAPI->search_instance_add_ex(patternMatcher,
                (char  *)patternList[i].data,
                patternList[i].length,
                &patternList[i],
                STR_SEARCH_CASE_SENSITIVE);
    }

    _dpd.searchAPI->search_instance_prep(patternMatcher);

    return patternMatcher;
}

typedef struct fieldPatternData_t
{
    const uint8_t *payload;
    unsigned length;
    httpSession *hsession;
} FieldPatternData;

static int http_field_pattern_match(void *id, void *unused_tree, int index, void *data, void *unused_neg)
{
    static const uint8_t crlf[] = "\r\n";
    static unsigned crlfLen = sizeof(crlf)-1;
    FieldPatternData *pFieldData = (FieldPatternData*)data;
    FieldPattern *target = (FieldPattern  *)id;
    const uint8_t* p;
    unsigned fieldOffset = target->length + index;
    unsigned remainingLength = pFieldData->length - fieldOffset;

    if (!(p = (uint8_t *)service_strstr(&pFieldData->payload[fieldOffset], remainingLength, crlf, crlfLen)))
    {
        return 1;
    }
    pFieldData->hsession->fieldOffset[target->patternType] = (uint16_t)fieldOffset;
    pFieldData->hsession->fieldEndOffset[target->patternType] = p - pFieldData->payload;
    return 1;
}

void httpGetNewOffsetsFromPacket(SFSnortPacket *pkt, httpSession *hsession, tAppIdConfig *pConfig)
{
#define MIN_HTTP_REQ_HEADER_SIZE (sizeof("GET /\r\n\r\n")-1)
    static const uint8_t crlfcrlf[] = "\r\n\r\n";
    static unsigned crlfcrlfLen = sizeof(crlfcrlf)-1;
    tDetectorHttpConfig *pHttpConfig = &pConfig->detectorHttpConfig;
    const uint8_t* p;
    uint8_t *headerEnd;
    PatternType fieldId;
    FieldPatternData patternMatchData;

    for (fieldId = AGENT_PT; fieldId <= COOKIE_PT; fieldId++)
    {
        // clear offset for default case;
        hsession->fieldOffset[fieldId] = 0;
    }

    if (!pkt || !pkt->payload || pkt->payload_size < MIN_HTTP_REQ_HEADER_SIZE)
        return;

    p = pkt->payload;

    patternMatchData.hsession = hsession;
    patternMatchData.payload = p;

    if (!(headerEnd = (uint8_t *)service_strstr(p, pkt->payload_size, crlfcrlf, crlfcrlfLen)))
        return;


    headerEnd += crlfcrlfLen;
    patternMatchData.length = (unsigned)(headerEnd - p);

    _dpd.searchAPI->search_instance_find_all(pHttpConfig->field_matcher,
                   (char *)p,
                   patternMatchData.length, 0,
                   &http_field_pattern_match,
                   (void *)(&patternMatchData));

}

int http_detector_finalize(tAppIdConfig *pConfig)
{
    size_t upc = 0;
    size_t apc = 0;
    size_t ctc = 0;
    size_t vpc = 0;

    tDetectorHttpConfig *pHttpConfig = &pConfig->detectorHttpConfig;
    HttpPatternLists* patternLists = &pConfig->httpPatternLists;
    u_int32_t numPatterns;

    /*create via pattern matcher */
    numPatterns = sizeof(via_http_detector_patterns)/sizeof(*via_http_detector_patterns);
    pHttpConfig->via_matcher = processPatterns(via_http_detector_patterns, numPatterns, &vpc, NULL);
    if (!pHttpConfig->via_matcher) return -1;

    /*create url pattern matcher */
    pHttpConfig->url_matcher = processPatterns(NULL, 0, &upc,
                             patternLists->urlPatternList);
    if (!pHttpConfig->url_matcher) return -1;

    /*create client agent pattern matcher */
    numPatterns = sizeof(client_agent_patterns)/sizeof(*client_agent_patterns);
    pHttpConfig->client_agent_matcher = processPatterns(client_agent_patterns,numPatterns, &apc,
                               patternLists->clientAgentPatternList);
    if (!pHttpConfig->client_agent_matcher) return -1;

    numPatterns = sizeof(header_patterns)/sizeof(*header_patterns);
    pHttpConfig->header_matcher = registerHeaderPatterns(header_patterns,numPatterns);
    if (!pHttpConfig->header_matcher) return -1;

    numPatterns = sizeof(host_payload_http_detector_patterns)/sizeof(*host_payload_http_detector_patterns);
    if (processHostPatterns(host_payload_http_detector_patterns, numPatterns, patternLists->hostPayloadPatternList, &patternLists->appUrlList, &patternLists->RTMPUrlList, pHttpConfig) < 0)
        return -1;

    numPatterns = sizeof(content_type_patterns)/sizeof(*content_type_patterns);
    pHttpConfig->content_type_matcher = processContentTypePatterns(content_type_patterns, numPatterns, patternLists->contentTypePatternList, &ctc);
    if (!pHttpConfig->content_type_matcher)
        return -1;

    numPatterns = sizeof(http_field_patterns)/sizeof(*http_field_patterns);
    pHttpConfig->field_matcher = processHttpFieldPatterns(http_field_patterns, numPatterns);
    if (!pHttpConfig->field_matcher)
        return -1;

    if (!processCHPList(patternLists->chpList, pHttpConfig)) return -1;

    pHttpConfig->chp_user_agent_matcher = pHttpConfig->chp_matchers[AGENT_PT];
    pHttpConfig->chp_host_matcher = pHttpConfig->chp_matchers[HOST_PT];
    pHttpConfig->chp_referer_matcher = pHttpConfig->chp_matchers[REFERER_PT];
    pHttpConfig->chp_uri_matcher = pHttpConfig->chp_matchers[URI_PT];
    pHttpConfig->chp_cookie_matcher = pHttpConfig->chp_matchers[COOKIE_PT];
    pHttpConfig->chp_req_body_matcher = pHttpConfig->chp_matchers[REQ_BODY_PT];
    pHttpConfig->chp_content_type_matcher = pHttpConfig->chp_matchers[CONTENT_TYPE_PT];
    pHttpConfig->chp_location_matcher = pHttpConfig->chp_matchers[LOCATION_PT];
    pHttpConfig->chp_body_matcher = pHttpConfig->chp_matchers[BODY_PT];

    return 0;
}

void http_detector_clean(tDetectorHttpConfig *pHttpConfig)
{
    int i;
    for (i = 0; i < sizeof(pHttpConfig->chp_matchers)/sizeof(pHttpConfig->chp_matchers[0]); i++)
    {
        _dpd.searchAPI->search_instance_free(pHttpConfig->chp_matchers[i]);
        pHttpConfig->chp_matchers[i] = NULL;
    }
    // These are same as chp_matchers[] and freed in the loop above
    pHttpConfig->chp_user_agent_matcher = pHttpConfig->chp_host_matcher =
        pHttpConfig->chp_referer_matcher = pHttpConfig->chp_uri_matcher =
        pHttpConfig->chp_cookie_matcher = pHttpConfig->chp_req_body_matcher =
        pHttpConfig->chp_content_type_matcher = pHttpConfig->chp_location_matcher =
        pHttpConfig->chp_body_matcher = NULL;

    if (pHttpConfig->via_matcher)
    {
        _dpd.searchAPI->search_instance_free(pHttpConfig->via_matcher);
        pHttpConfig->via_matcher = NULL;
    }
    if (pHttpConfig->url_matcher)
    {
        _dpd.searchAPI->search_instance_free(pHttpConfig->url_matcher);
        pHttpConfig->url_matcher = NULL;
    }
    if (pHttpConfig->client_agent_matcher)
    {
        _dpd.searchAPI->search_instance_free(pHttpConfig->client_agent_matcher);
        pHttpConfig->client_agent_matcher = NULL;
    }
    if (pHttpConfig->header_matcher)
    {
        _dpd.searchAPI->search_instance_free(pHttpConfig->header_matcher);
        pHttpConfig->header_matcher = NULL;
    }
    if (pHttpConfig->content_type_matcher)
    {
        _dpd.searchAPI->search_instance_free(pHttpConfig->content_type_matcher);
        pHttpConfig->content_type_matcher = NULL;
    }
    if (pHttpConfig->field_matcher)
    {
        _dpd.searchAPI->search_instance_free(pHttpConfig->field_matcher);
        pHttpConfig->field_matcher = NULL;
    }

    destroyHostUrlMatcher(&pHttpConfig->hostUrlMatcher);
    destroyHostUrlMatcher(&pHttpConfig->RTMPHostUrlMatcher);
    destroyHostUrlPatternList(&pHttpConfig->hostUrlPatternsList);
}

static inline void FreeMatchStructures(MatchedPatterns *mp)
{
    MatchedPatterns *tmp;

    while(mp)
    {
        tmp = mp;
        mp = mp->next;
        free(tmp);
    }
}

static void rewriteCHP (const char *buf, int bs, int start,
                        int psize, char *adata, char **outbuf,
                        int insert)
{
    int maxs, bufcont, as;
    char *copyPtr;

    // special behavior for insert vs. rewrite
    if (insert)
    {
        // we don't want to insert a string that is already present
        if (!adata || _dpd.SnortStrcasestr((const char *)buf, bs, adata))
            return;

        start += psize;
        bufcont = start;
        as = strlen(adata);
        maxs = bs+as;
    }
    else
    {
        if (adata)
        {
            // we also don't want to replace a string with an identical one.
            if (!strncmp(buf+start,adata,psize))
                return;

            as = strlen(adata);
        }
        else
            as = 0;

        bufcont = start+psize;
        maxs = bs+(as-psize);
    }

    *outbuf = copyPtr = (char *)calloc(maxs+1,sizeof(char));
    if (!copyPtr)
        return;

    memcpy(copyPtr, buf, start);
    copyPtr += start;
    if (adata)
    {
        memcpy(copyPtr, adata, as);
        copyPtr += as;
    }
    memcpy(copyPtr, buf+bufcont, bs-bufcont);
}

static char * normalize_userid (char *user)
{
    int i, old_size;
    int percent_count = 0;
    char a, b;
    char *tmp_ret, *tmp_user;

    old_size = strlen(user);

    // find number of '%'
    for (i = 0; i < old_size; i++)
    {
        if (*(user+i) == '%')
            percent_count++;
    }
    if (0 == percent_count)
    {
        /* no change allows an early out */
        return user;
    }

    /* Shrink user string in place */
    tmp_ret = user;
    tmp_user = user;

    while (*tmp_user)
    {
        if ((*tmp_user == '%') &&
            ((a = tmp_user[1]) && (b = tmp_user[2])) &&
             (isxdigit(a) && isxdigit(b)))
        {
            if (a >= 'a')
                a -= 'a'-'A';

            if (a >= 'A')
                a -= ('A' - 10);
            else
                a -= '0';

            if (b >= 'a')
                b -= 'a'-'A';

            if (b >= 'A')
                b -= ('A' - 10);
            else
                b -= '0';

            *tmp_ret++ = 16*a+b;
             tmp_user+=3;
        } else {
            *tmp_ret++ = *tmp_user++;
        }
    }
    *tmp_ret++ = '\0';

    return user;
}

static void extractCHP (char *buf, int bs, int start,
                        int psize, char *adata,
                        char **outbuf)
{
    char *begin = buf+start+psize;
    char *end = NULL;
    char *tmp;
    int i, as;

    if (adata)
        as = strlen(adata);
    else
        as = 0;

    // find where the pattern ends so we can allocate a buffer
    for (i = 0; i < as; i++)
    {
        tmp = strchr(begin, *(adata+i));
        if (tmp)
        {
            if (!end || (tmp < end))
                end = tmp;
        }
    }
    if (!end)
    {
        if ((tmp = strchr(begin, 0x0d)))
        {
            end = tmp;
        }
        if ((tmp = strchr(begin, 0x0a)))
        {
            if (!end || (tmp < end))
                end = tmp;
        }
    }

    if (!end)
        end = begin+bs;

    *outbuf = strndup(begin, end-begin);
}

static uint32_t ddToIp (char *start, int size)
{
    uint32_t ret_addr = 0;
    char *p;
    int tmp = 0;
    int octet = 3;
    int digit_count = 1;
    int done = 0;

    for (p = start;
         p < start+size;
         p++)
    {
        if (isdigit(*p))
        {
            // if there are more than three digits in a row
            if (digit_count > 3)
            {
                // this might be a spurrious digit after the IP address
                if (octet == 0 && tmp && tmp <= 255)
                {
                    ret_addr += tmp;
                    done = 1;
                    break;
                }
                else
                    return 0;
            }
            // otherwise, increase the value of tmp
            tmp *= 10;
            tmp += *p - '0';
            digit_count++;
        }
        // 0x2e is '.'
        else if (*p == 0x2e)
        {
            // make sure we don't have random dots in there
            if (!tmp)
                return 0;
            // otherwise, increase the return value
            else
            {
                // octet value must fit in 8-bit boundary
                if (tmp > 255)
                    return 0;
                ret_addr += tmp <<octet*8;
                //maybe this is an extraneous '.' at the end
                if (octet == 0)
                {
                    done = 1;
                    break;
                }
                octet--;
                digit_count = 1;
                tmp = 0;
            }
        }
        // this might be a character right after the IP address
        else if (octet == 0 && tmp && tmp <= 255)
        {
            ret_addr += tmp;
            done = 1;
            break;
        }
        // bail out if we see something funny
        else
            return 0;
    }
    if (octet || tmp > 255)
        return 0;
    if (!done)
        ret_addr += tmp;
    return htonl(ret_addr);
}

static uint32_t ffSetIp (char *buf, int buf_size, int start, int psize)
{
    uint32_t ret_address;

    ret_address = ddToIp(buf+start+psize, buf_size);

    return ret_address;
}

static uint16_t ffSetPort (char *buf, int buf_size, int start, int psize)
{
    uint16_t temp_port = 0;
    uint16_t new_digit;
    char *p;
    int i;

    for (p = buf+start+psize, i = 1; p < buf+buf_size && isdigit(*p); p++, i++)
    {
        new_digit = *p -'0';
        // we don't want to try to put a value gt 65535 into a uint_16t
        if ((i > 5) || (temp_port > 6535 || (temp_port == 6535 && new_digit > 5))) return 0;
        temp_port *= 10;
        temp_port += *p - '0';
    }

    return temp_port;
}

static uint8_t ffSetProtocol (char *buf, int buf_size, int start, int psize)
{
    uint8_t temp_protocol = 0;
    uint8_t new_digit;
    char *p;
    int i;

    for (p = buf+start+psize, i = 1; p < buf+buf_size && isdigit(*p); p++, i++)
    {
        new_digit = *p - '0';
        // we don't want to try to put a value gt 255 into a uint8_t
        if ((i > 3) || (temp_protocol > 25 || (temp_protocol == 25 && new_digit > 5))) return 0;
        temp_protocol *= 10;
        temp_protocol += new_digit;
    }

    return temp_protocol;
}

static void fflowCreate (char *adata, fflow_info *fflow,
                          SFSnortPacket *p, tAppId target_appid)
{
    char *saddr_string = NULL;
    char *daddr_string = NULL;
    char *sport_string = NULL;
    char *dport_string = NULL;
    char *protocol_string = NULL;
    char *appid = NULL;
    sfaddr_t *sip, *dip;
    int temp_port = 0;

    /*
       The Action Data for this action is special
       THE SEQUENCE MUST BE
       source_address source_port dest_address dest_port protocol appid
       DELIMITED BY A SPACE
       if any value is '*', that means we should have already set this value with a previous action
    */
    if (!(saddr_string = strtok(adata, " "))) return;
    if (!(sport_string = strtok(NULL, " "))) return;
    if (!(daddr_string = strtok(NULL, " "))) return;
    if (!(dport_string = strtok(NULL, " "))) return;
    if (!(protocol_string = strtok(NULL, " "))) return;
    if (!(appid = strtok(NULL, " "))) return;

    switch (*saddr_string)
    {
    case 'S':
        sip = GET_SRC_IP(p);
        fflow->sip = sfaddr_get_ip4_value(sip);
        break;
    case 'D':
        sip = GET_DST_IP(p);
        fflow->sip = sfaddr_get_ip4_value(sip);
        break;
    case '0':
        sip = 0;
        break;
    case '*':
        if (!fflow->sip)
            return;
        break;
    default:
        if ((!fflow->sip) && (!(fflow->sip = ddToIp(saddr_string, strlen(saddr_string)))))
            return;
    }

    switch (*sport_string)
    {
    case 'S':
        if (strlen(sport_string) > 2)
        {
            if ((temp_port = strtol(sport_string+1, NULL, 10)))
                fflow->sport = p->src_port + temp_port;
            else
                return;
        }
        else
            fflow->sport = p->src_port;
        break;
    case 'D':
        if (strlen(sport_string) > 2)
        {
            if ((temp_port = strtol(sport_string+1, NULL, 10)))
                fflow->sport = p->dst_port + temp_port;
            else
                return;
        }
        else
            fflow->sport = p->dst_port;
        break;
    case '0':
        fflow->sport = 0;
        break;
    case '*':
        if (!fflow->sport)
            return;
        break;
    default:
        if ((!fflow->sport) && (!(fflow->sport = ffSetPort(sport_string, strlen(sport_string), 0, 0))))
            return;
    }

    switch (*daddr_string)
    {
    case 'S':
        dip = GET_SRC_IP(p);
        fflow->dip = sfaddr_get_ip4_value(dip);
        break;
    case 'D':
        dip = GET_DST_IP(p);
        fflow->dip = sfaddr_get_ip4_value(dip);
        break;
    case '0':
        fflow->dip = 0;
        break;
    case '*':
        if (!fflow->dip)
            return;
        break;
    default:
        if ((!fflow->dip) && (!(fflow->dip = ddToIp(daddr_string, strlen(daddr_string)))))
            return;
    }

    switch (*dport_string)
    {
    case 'S':
        if (strlen(dport_string) > 2)
        {
            if ((temp_port = strtol(dport_string+1, NULL, 10)))
                fflow->dport = p->src_port + temp_port;
            else
                return;
        }
        else
            fflow->dport = p->src_port;
        break;
    case 'D':
        if (strlen(dport_string) > 2)
        {
            if ((temp_port = strtol(dport_string+1, NULL, 10)))
                fflow->dport = p->dst_port + temp_port;
            else
                return;
        }
        else
            fflow->dport = p->dst_port;
        break;
    case '0':
        fflow->dport = 0;
        break;
    case '*':
        if (!fflow->dport)
            return;
        break;
    default:
        if ((!fflow->dport) && (!(fflow->dport = ffSetPort(dport_string, strlen(dport_string), 0, 0))))
            return;
    }

    switch (*protocol_string)
    {
    case 'T':
        fflow->protocol = IPPROTO_TCP;
        break;
    case 'U':
        fflow->protocol = IPPROTO_UDP;
        break;
    case '0':
        fflow->protocol = 0;
        break;
    case 'S':
    case 'D':
        fflow->protocol = IsTCP(p) ? IPPROTO_TCP : IPPROTO_UDP;
        break;
    case '*':
        if (!fflow->protocol)
            return;
        break;
    default:
        if ((!fflow->protocol) && (!(fflow->protocol = ffSetProtocol(protocol_string, strlen(protocol_string), 0, 0))))
            return;
    }

    switch (*appid)
    {
    case '*':
        fflow->appId = target_appid;
        break;
    default:
        fflow->appId = strtol(appid, NULL, 10);
    }

    fflow->flow_prepared = 1;
}

void finalizeFflow (fflow_info *fflow, unsigned app_type_flags, tAppId target_appId, SFSnortPacket *p)
{
    tAppIdData *fp;
    sfaddr_t saddr, daddr;

    sfip_set_raw(&saddr, &fflow->sip, AF_INET);
    sfip_set_raw(&daddr, &fflow->dip, AF_INET);

    if (!(fp = AppIdEarlySessionCreate(NULL, p, &saddr, fflow->sport, &daddr, fflow->dport,
                    fflow->protocol, target_appId, 0)))
        return;

    if (app_type_flags & APP_TYPE_SERVICE)
    {
        fp->serviceAppId = target_appId;
        fp->rnaServiceState = RNA_STATE_FINISHED;
        fp->rnaClientState = RNA_STATE_FINISHED;
    }
    if (app_type_flags & APP_TYPE_CLIENT)
    {
        fp->clientAppId = target_appId;
        fp->rnaClientState = RNA_STATE_FINISHED;
    }
    if (app_type_flags & APP_TYPE_PAYLOAD)
    {
        fp->payloadAppId = target_appId;
    }

}
int scanKeyCHP (PatternType ptype, char *buf, int buf_size, CHPMatchTally **ppTally, MatchedCHPAction **ppmatches, const tDetectorHttpConfig *pHttpConfig)
{
    CHPTallyAndActions tallyAndActions;
    tallyAndActions.pTally = *ppTally;
    tallyAndActions.matches = *ppmatches;

    _dpd.searchAPI->search_instance_find_all(pHttpConfig->chp_matchers[ptype],
               (char *)buf,
               buf_size, 0,
               &chp_key_pattern_match,
               (void *)(&tallyAndActions));

    *ppTally = tallyAndActions.pTally;
    *ppmatches = tallyAndActions.matches;
    return (int)(tallyAndActions.pTally != NULL);
}

tAppId scanCHP (PatternType ptype, char *buf, int buf_size, MatchedCHPAction *mp,
                char **version, char **user, char **new_field,
                int *total_found, httpSession *hsession, SFSnortPacket *p, const tDetectorHttpConfig *pHttpConfig)
{
    MatchedCHPAction *second_sweep_for_inserts = NULL;
    int do_not_further_modify_field = 0;
    CHPAction *match = NULL;
    tAppId ret = APP_ID_NONE;
    MatchedCHPAction *tmp;

    if (ptype > MAX_KEY_PATTERN)
    {
        // There is no previous attempt to match generated by scanKeyCHP()
        mp = NULL;

        _dpd.searchAPI->search_instance_find_all(pHttpConfig->chp_matchers[ptype],
                   (char *)buf,
                   buf_size, 0,
                   &chp_pattern_match,
                   (void *)(&mp));
    }
    if (!mp)
        return APP_ID_NONE;

    if (appidStaticConfig->disable_safe_search)
    {
        new_field = NULL;
    }

    for(tmp = mp; tmp; tmp = tmp->next)
    {
        match = (CHPAction *)tmp->mpattern;
        if (match->appIdInstance > hsession->chp_candidate)
            break; // because the list is sorted we know there are no more
        else if (match->appIdInstance == hsession->chp_candidate)
        {
            switch (match->action)
            {
                case DEFER_TO_SIMPLE_DETECT:
                    // Ignore all other patterns; we are done.
                    FreeMatchedCHPActions(mp);
                    // Returning APP_ID_NONE will trigger the clearing of hsession->skip_simple_detect
                    // and the freeing of any planned field rewrites.
                    return APP_ID_NONE;
                default:
                    (*total_found)++;
                    break;
                case ALTERNATE_APPID: // an "optional" action that doesn't count towards totals
                case REWRITE_FIELD:   // handled when the action completes successfully
                case INSERT_FIELD:    // handled when the action completes successfully
                    break;
            }
            if (!ret)
                ret = hsession->chp_candidate;
        }
        else
            continue; // keep looking

        switch (match->action)
        {
        case COLLECT_VERSION:
            if (!*version)
                extractCHP(buf, buf_size, tmp->index, match->psize,
                       match->action_data, version);
            hsession->skip_simple_detect = true;
            break;
        case EXTRACT_USER:
            if (!*user && !appidStaticConfig->chp_userid_disabled)
            {
                extractCHP(buf, buf_size, tmp->index, match->psize,
                       match->action_data, user);
                if (*user)
                    *user = normalize_userid(*user);
            }
            break;
        case REWRITE_FIELD:
            if (!do_not_further_modify_field &&
                NULL != new_field &&
                NULL == *new_field)
            {
                // The field supports rewrites, and a rewrite hasn't happened.
                rewriteCHP(buf, buf_size, tmp->index, match->psize,
                       match->action_data, new_field, 0);
                (*total_found)++;
                do_not_further_modify_field = 1;
            }
            break;
        case FUTURE_APPID_SESSION_SIP:
            if (appidStaticConfig->chp_fflow_disabled)
                break;
            if (!hsession->fflow)
            {
                if (!(hsession->fflow = (fflow_info*)_dpd.snortAlloc(1,
                    sizeof(fflow_info), PP_APP_ID, PP_MEM_CATEGORY_SESSION)))
                {
                    appidStaticConfig->chp_fflow_disabled = 1;
                    break;
                }
            }
            if (!hsession->fflow->sip)
                hsession->fflow->sip = ffSetIp(buf, buf_size, tmp->index, match->psize);
            break;
        case FUTURE_APPID_SESSION_DIP:
            if (appidStaticConfig->chp_fflow_disabled)
                break;
            if (!hsession->fflow)
            {
                if (!(hsession->fflow = (fflow_info*)_dpd.snortAlloc(1,
                    sizeof(fflow_info), PP_APP_ID, PP_MEM_CATEGORY_SESSION)))
                {
                    appidStaticConfig->chp_fflow_disabled = 1;
                    break;
                }
            }
            if (!hsession->fflow->dip)
                hsession->fflow->dip = ffSetIp(buf, buf_size, tmp->index, match->psize);
            break;
        case FUTURE_APPID_SESSION_SPORT:
            if (appidStaticConfig->chp_fflow_disabled)
                break;
            if (!hsession->fflow)
            {
                if (!(hsession->fflow = (fflow_info*)_dpd.snortAlloc(1,
                     sizeof(fflow_info), PP_APP_ID, PP_MEM_CATEGORY_SESSION)))
                {
                    appidStaticConfig->chp_fflow_disabled = 1;
                    break;
                }
            }
            if (!hsession->fflow->sport)
                hsession->fflow->sport = ffSetPort(buf, buf_size, tmp->index, match->psize);
            break;
        case FUTURE_APPID_SESSION_DPORT:
            if (appidStaticConfig->chp_fflow_disabled)
                break;
            if (!hsession->fflow)
            {
                if (!(hsession->fflow = (fflow_info*)_dpd.snortAlloc(1,
                    sizeof(fflow_info), PP_APP_ID, PP_MEM_CATEGORY_SESSION)))
                {
                    appidStaticConfig->chp_fflow_disabled = 1;
                    break;
                }
            }
            if (!hsession->fflow->dport)
                hsession->fflow->dport = ffSetPort(buf, buf_size, tmp->index, match->psize);
            break;
        case FUTURE_APPID_SESSION_PROTOCOL:
            if (appidStaticConfig->chp_fflow_disabled)
                break;
            if (!hsession->fflow)
            {
                if (!(hsession->fflow = (fflow_info*)_dpd.snortAlloc(1,
                    sizeof(fflow_info), PP_APP_ID, PP_MEM_CATEGORY_SESSION)))
                {
                    appidStaticConfig->chp_fflow_disabled = 1;
                    break;
                }
            }
            if (!hsession->fflow->protocol)
                hsession->fflow->protocol = ffSetProtocol(buf, buf_size, tmp->index, match->psize);
            break;
        case FUTURE_APPID_SESSION_CREATE:
            if (appidStaticConfig->chp_fflow_disabled)
                break;
            if (!hsession->fflow)
            {
                if (!(hsession->fflow = (fflow_info*)_dpd.snortAlloc(1,
                    sizeof(fflow_info), PP_APP_ID, PP_MEM_CATEGORY_SESSION)))
                {
                    appidStaticConfig->chp_fflow_disabled = 1;
                    break;
                }
            }
            fflowCreate(match->action_data, hsession->fflow, p, hsession->chp_candidate);
            break;
        case INSERT_FIELD:
            if (!do_not_further_modify_field && second_sweep_for_inserts == NULL)
            {
                if (match->action_data)
                {
                    // because this insert is the first one we have come across
                    // we only need to remember this ONE for later.
                    second_sweep_for_inserts = tmp;
                }
                else
                {
                    // This is an attempt to "insert nothing"; call it a match
                    // The side effect is to set the do_not_further_modify_field to 1 (true)

                    // Note that an attempt to "rewrite with identical string"
                    // is NOT equivalent to an "insert nothing" because of case-
                    //  insensitive pattern matching

                    do_not_further_modify_field = 1;
                    (*total_found)++;
                }
            }
            break;
        case ALTERNATE_APPID:
            hsession->chp_alt_candidate = strtol(match->action_data, NULL, 10);
            hsession->skip_simple_detect = true;
            break;
        case HOLD_FLOW:
            hsession->chp_hold_flow = 1;
            break;
        case GET_OFFSETS_FROM_REBUILT:
            hsession->get_offsets_from_rebuilt = 1;
            hsession->chp_hold_flow = 1;
            break;
        case SEARCH_UNSUPPORTED:
        case NO_ACTION:
            hsession->skip_simple_detect = true;
            break;
        default:
            break;
        }
    }
    // non-NULL second_sweep_for_inserts indicates the insert action we will use.
    if (!do_not_further_modify_field && second_sweep_for_inserts &&
        NULL != new_field &&
        NULL == *new_field)
    {
        // We will take the first INSERT_FIELD with an action string,
        // which was decided with the setting of second_sweep_for_inserts.
        rewriteCHP(buf, buf_size, second_sweep_for_inserts->index,
                second_sweep_for_inserts->mpattern->psize,
                second_sweep_for_inserts->mpattern->action_data,
                new_field, 1); // insert
        (*total_found)++;
    }

    FreeMatchedCHPActions(mp);
    return ret;
}

static inline int optionallyReplaceWithStrdup(char **optionalStr, const char *strToDup)
{
    if (optionalStr)
    {
        if (*optionalStr)
            free(*optionalStr);

        if (NULL == (*optionalStr = strdup(strToDup)))
            return -1; // malloc failed, if they care.
    }
    return 0; // success
}

void identifyUserAgent(const u_int8_t *start, int size, tAppId *serviceAppId, tAppId *clientAppId, char **version,
                       const tDetectorHttpConfig *pHttpConfig)
{
    int skypeDetect;
    int mobileDetect;
    int safariDetect;
    unsigned int appleEmailDetect;
    int firefox_detected, android_browser_detected;
    int dominant_pattern_detected;
    int longest_misc_match;
    const u_int8_t *end;
    MatchedPatterns *mp = NULL;
    MatchedPatterns *tmp;
    DetectorHTTPPattern *match;
    u_int8_t* buffPtr;
    unsigned int i;
    char temp_ver[MAX_VERSION_SIZE];
    temp_ver[0] = 0;

    _dpd.searchAPI->search_instance_find_all(pHttpConfig->client_agent_matcher,
               (char *)start,
               size,
               0,
               &http_pattern_match, (void *)&mp);


    if (mp)
    {
        end = start + size;
        temp_ver[0] = 0;
        skypeDetect = 0;
        mobileDetect = 0;
        safariDetect = 0;
        firefox_detected = 0;
        android_browser_detected = 0;
        dominant_pattern_detected = 0;
        longest_misc_match = 0;
        i = 0;
        *clientAppId = APP_ID_NONE;
        *serviceAppId = APP_ID_HTTP;
        for(tmp = mp; tmp; tmp = tmp->next)
        {
            match = (DetectorHTTPPattern *)tmp->mpattern;
            switch (match->client_app)
            {
            case APP_ID_INTERNET_EXPLORER:
            case APP_ID_FIREFOX:
                if (dominant_pattern_detected)
                    break;
                buffPtr = (u_int8_t*) start + tmp->index + match->pattern_size;
                if (*buffPtr != ' ' && *buffPtr != 0x09 && *buffPtr != '/')
                    break;
                buffPtr++;
                while (i < MAX_VERSION_SIZE-1 && buffPtr < end)
                {
                    if(*buffPtr != ' ' && *buffPtr != 0x09 && *buffPtr != ';' && *buffPtr != ')')
                        temp_ver[i++] = *buffPtr++;
                    else
                        break;
                }
                if(i == 0)
                    break;

                temp_ver[i] = 0;

                /*compatibility check */
                if (match->client_app == APP_ID_INTERNET_EXPLORER
                        && strstr((char *)buffPtr, "SLCC2"))
                {
                    if ((MAX_VERSION_SIZE-i) >= (sizeof(COMPATIBLE_BROWSER_STRING) - 1))
                    {
                        strcat(temp_ver, COMPATIBLE_BROWSER_STRING);
                    }
                }
                // Pick firefox over some things, but pick a misc app over Firefox.
                if (match->client_app == APP_ID_FIREFOX)
                    firefox_detected = 1;
                *serviceAppId = APP_ID_HTTP;
                *clientAppId = match->client_app;
                break;

            case APP_ID_CHROME:
                if (dominant_pattern_detected)
                    break;
                buffPtr = (u_int8_t*) start + tmp->index + match->pattern_size;
                if (*buffPtr != ' ' && *buffPtr != 0x09 && *buffPtr != '/')
                    break;
                buffPtr++;
                while (i < MAX_VERSION_SIZE-1 && buffPtr < end)
                {
                    if(*buffPtr != ' ' && *buffPtr != 0x09 && *buffPtr != ';' && *buffPtr != ')')
                        temp_ver[i++] = *buffPtr++;
                    else
                        break;
                }
                if(i == 0)
                    break;

                dominant_pattern_detected = 1;
                temp_ver[i] = 0;
                *serviceAppId = APP_ID_HTTP;
                *clientAppId = match->client_app;
                break;

            case APP_ID_ANDROID_BROWSER:
                if (dominant_pattern_detected)
                    break;
                buffPtr = (u_int8_t*) start + tmp->index + match->pattern_size;
                if (*buffPtr != ' ' && *buffPtr != 0x09 && *buffPtr != '/')
                    break;
                buffPtr++;
                while (i < MAX_VERSION_SIZE-1 && buffPtr < end)
                {
                    if(*buffPtr != ' ' && *buffPtr != 0x09 && *buffPtr != ';' && *buffPtr != ')')
                        temp_ver[i++] = *buffPtr++;
                    else
                        break;
                }
                if(i == 0)
                    break;

                temp_ver[i] = 0;
                android_browser_detected = 1;
                break;

            case APP_ID_KONQUEROR:
            case APP_ID_CURL:
            case APP_ID_PICASA:
                if (dominant_pattern_detected)
                    break;
            case APP_ID_WINDOWS_MEDIA_PLAYER:
            case APP_ID_BITTORRENT:
                buffPtr = (u_int8_t*) start + tmp->index + match->pattern_size;
                if (*buffPtr != ' ' && *buffPtr != 0x09 && *buffPtr != '/')
                    break;
                buffPtr++;
                while (i < MAX_VERSION_SIZE-1 && buffPtr < end)
                {
                    if(*buffPtr != ' ' && *buffPtr != 0x09 && *buffPtr != ';' && *buffPtr != ')')
                        temp_ver[i++] = *buffPtr++;
                    else
                        break;
                }
                if(i == 0)
                    break;

                temp_ver[i] = 0;
                *serviceAppId = APP_ID_HTTP;
                *clientAppId = match->client_app;
                goto done;

            case APP_ID_GOOGLE_DESKTOP:
                buffPtr = (u_int8_t*) start + tmp->index + match->pattern_size;
                if (*buffPtr != ')')
                {
                    if (*buffPtr != ' ' && *buffPtr != 0x09 && *buffPtr != '/')
                        break;
                    buffPtr++;
                    while (i < MAX_VERSION_SIZE-1 && buffPtr < end)
                    {
                        if(*buffPtr != ' ' && *buffPtr != 0x09 && *buffPtr != ';')
                            temp_ver[i++] = *buffPtr++;
                        else
                            break;
                    }
                    if(i == 0)
                        break;
                    temp_ver[i] = 0;
                }
                *serviceAppId = APP_ID_HTTP;
                *clientAppId = match->client_app;
                goto done;

            case APP_ID_SAFARI_MOBILE_DUMMY:
                mobileDetect = 1;
                break;

            case APP_ID_SAFARI:
                if (dominant_pattern_detected)
                    break;
                safariDetect = 1;
                break;

            case APP_ID_APPLE_EMAIL:
                appleEmailDetect = 1;
                for (i = 0; i < 3 && appleEmailDetect; i++)
                {
                    buffPtr = (u_int8_t*) strstr((char *)start, (char *)APPLE_EMAIL_PATTERNS[i]);
                    appleEmailDetect  = ((u_int8_t*)buffPtr && (i != 0 || buffPtr == ((u_int8_t*)start)));
                }
                if (appleEmailDetect)
                {
                    dominant_pattern_detected = !(buffPtr && strstr((char *)buffPtr, SAFARI_PATTERN) != NULL);
                    temp_ver[0] = 0;
                    *serviceAppId = APP_ID_HTTP;
                    *clientAppId = match->client_app;
                }
                i = 0;
                break;

            case APP_ID_WGET:
                buffPtr = (u_int8_t*) start + tmp->index + match->pattern_size;
                while (i < MAX_VERSION_SIZE - 1 && buffPtr < end)
                {
                    temp_ver[i++] = *buffPtr++;
                }
                temp_ver[i] = 0;
                *serviceAppId = APP_ID_HTTP;
                *clientAppId = match->client_app;
                goto done;

            case APP_ID_BLACKBERRY_BROWSER:
                while( start < end && *start != '/' ) start++;
                if(start >= end)
                    break;
                start++;
                while (i < MAX_VERSION_SIZE -1 && start < end)
                {
                    if(*start != ' ' && *start != 0x09 && *start != ';')
                        temp_ver[i++] = *start++;
                    else
                        break;
                }
                if(i == 0)
                    break;
                temp_ver[i] = 0;

                *serviceAppId = APP_ID_HTTP;
                *clientAppId = match->client_app;
                goto done;

            case APP_ID_SKYPE:
                skypeDetect  = 1;
                break;

            case APP_ID_HTTP:
                break;

            case APP_ID_OPERA:
                *serviceAppId = APP_ID_HTTP;
                *clientAppId = match->client_app;
                break;

            case FAKE_VERSION_APP_ID:
                if (temp_ver[0])
                {
                    temp_ver[0] = 0;
                    i = 0;
                }
                buffPtr = (u_int8_t*) start + tmp->index + match->pattern_size;
                if (*buffPtr == (u_int8_t)'/')
                {
                    buffPtr++;
                    while (i < MAX_VERSION_SIZE - 1 && buffPtr < end)
                    {
                        if(*buffPtr != ' ' && *buffPtr != 0x09 && *buffPtr != ';' && *buffPtr != ')')
                            temp_ver[i++] = *buffPtr++;
                        else
                            break;
                    }
                }
                temp_ver[i] = 0;
                break;

            default:
                if (match->client_app)
                {
                    if (match->pattern_size <= longest_misc_match) break;
                    longest_misc_match = match->pattern_size;
                    i =0;
                    /* if we already collected temp_ver information after seeing 'Version', let's use that*/
                    buffPtr = (u_int8_t*) start + tmp->index + match->pattern_size;
                    /* we may have to enter the pattern with the / in it. */
                    if (*buffPtr == (u_int8_t)'/' || *buffPtr == (u_int8_t)' ')
                        buffPtr++;
                    if (buffPtr-1 > start && buffPtr < end && (*(buffPtr-1) == (u_int8_t)'/' || *(buffPtr-1) == (u_int8_t)' '))
                    {
                        while (i < MAX_VERSION_SIZE -1 && buffPtr < end)
                        {
                            if(*buffPtr != ' ' && *buffPtr != 0x09 && *buffPtr != ';' && *buffPtr != ')')
                                temp_ver[i++] = *buffPtr++;
                            else
                                break;
                        }
                        temp_ver[i] = 0;
                    }
                    dominant_pattern_detected = 1;
                    *serviceAppId = APP_ID_HTTP;
                    *clientAppId = match->client_app;
                }
            }
        }
        if (mobileDetect && safariDetect && !dominant_pattern_detected)
        {
            *serviceAppId = APP_ID_HTTP;
            *clientAppId = APP_ID_SAFARI_MOBILE;
        }
        else if (safariDetect && !dominant_pattern_detected)
        {
            *serviceAppId = APP_ID_HTTP;
            *clientAppId = APP_ID_SAFARI;
        }
        else if (firefox_detected && !dominant_pattern_detected)
        {
            *serviceAppId = APP_ID_HTTP;
            *clientAppId = APP_ID_FIREFOX;
        }
        else if (android_browser_detected && !dominant_pattern_detected)
        {
            *serviceAppId = APP_ID_HTTP;
            *clientAppId = APP_ID_ANDROID_BROWSER;
        }
        /* Better to choose Skype over any other ID  */
        else if (skypeDetect)
        {
            *serviceAppId = APP_ID_SKYPE_AUTH;
            *clientAppId = APP_ID_SKYPE;
        }
    }

done:
    optionallyReplaceWithStrdup(version,temp_ver);
    FreeMatchStructures(mp);
}

int getAppidByViaPattern(const uint8_t *data, unsigned size, char **version, const tDetectorHttpConfig *pHttpConfig)
{
    unsigned i;
    const uint8_t *data_ptr;
    const uint8_t *end = data + size;
    MatchedPatterns *mp = NULL;
    DetectorHTTPPattern *match = NULL;
    char temp_ver[MAX_VERSION_SIZE];

    if (pHttpConfig->via_matcher)
    {
        _dpd.searchAPI->search_instance_find_all(pHttpConfig->via_matcher,
               (char *)data,
               size, 0,
               &http_pattern_match, (void *)&mp);
    }

    if(mp)
    {
        match = (DetectorHTTPPattern *)mp->mpattern;
        switch (match->service_id)
        {
        case APP_ID_SQUID:
            data_ptr = (u_int8_t*) data + mp->index + match->pattern_size;
            if (*data_ptr == '/')
            {
                data_ptr++;
                for (i = 0;
                     data_ptr < end && i < (MAX_VERSION_SIZE-1) && *data_ptr != ')' && isprint(*data_ptr);
                     data_ptr++)
                {
                    temp_ver[i++] = (char)*data_ptr;
                }
            }
            else
                i = 0;
            temp_ver[i] = 0;
            optionallyReplaceWithStrdup(version,temp_ver);
            FreeMatchStructures(mp);
            return APP_ID_SQUID;

        default:
            FreeMatchStructures(mp);
            return APP_ID_NONE;
        }
    }
    return APP_ID_NONE;
}

#define HTTP_HEADER_WORKINGWITH_ASPROXY "ASProxy/"

tAppId scan_header_x_working_with(const uint8_t *data, uint32_t size, char **version)
{
    uint32_t i;
    const uint8_t *end;
    char temp_ver[MAX_VERSION_SIZE];

    temp_ver[0] = 0;

    if (size >= (sizeof(HTTP_HEADER_WORKINGWITH_ASPROXY)-1)
            && memcmp(data,HTTP_HEADER_WORKINGWITH_ASPROXY,sizeof(HTTP_HEADER_WORKINGWITH_ASPROXY)-1) == 0)
    {
        end = data+size;
        data += sizeof(HTTP_HEADER_WORKINGWITH_ASPROXY)-1;
        for (i = 0;
                data < end && i < (MAX_VERSION_SIZE-1) && *data != ')' && isprint(*data);
                data++)
        {
            temp_ver[i++] = (char)*data;
        }
        temp_ver[i] = 0;
        optionallyReplaceWithStrdup(version,temp_ver);
        return APP_ID_ASPROXY;
    }
    return APP_ID_NONE;
}
tAppId getAppidByContentType(const uint8_t *data, int size, const tDetectorHttpConfig *pHttpConfig)
{
    MatchedPatterns *mp = NULL;
    DetectorHTTPPattern *match;
    tAppId payloadId;

    if (pHttpConfig->content_type_matcher)
    {
        _dpd.searchAPI->search_instance_find_all(pHttpConfig->content_type_matcher,
                   (char *)data,
                   size, 0,
                   &content_pattern_match, (void *)&mp);
    }

    if (!mp)
        return APP_ID_NONE;

    match = mp->mpattern;
    payloadId = match->appId;

    FreeMatchStructures(mp);

    return payloadId;
}

static int http_header_pattern_match(void* id, void *unused_tree, int index, void* data, void *unused_neg)
{
    HeaderMatchedPatterns *matches = (HeaderMatchedPatterns *)data;
    HeaderPattern *target = (HeaderPattern *)id;
    HTTPHeaderIndices *headers = matches->headers;

    if (matches->last_match >= 0)
    {
        headers[matches->last_match].end = index;
        matches->last_match = -1;
    }

    if (target->id < HTTP_ID_LEN)
    {
        if (index == 0)
        {
            goto store_index;
        }
        else if (index == matches->last_index_end)
        {
            /* This checks if the last match was \r or \n */
            /* It is still possible to have nefarious payloads */
            /* that have HTTP Headers in "" */
            goto store_index;
        }
    }

    goto done;

store_index:
    headers[target->id].start = index + target->length;
    headers[target->id].end = 0;
    matches->last_match = target->id;
done:
    matches->last_index_end = index + target->length;
    return 0;
}

int getHTTPHeaderLocation(const uint8_t *data, unsigned size, HttpId id, int *start, int *end, HeaderMatchedPatterns *hmp,
                          const tDetectorHttpConfig *pHttpConfig)
{
    HTTPHeaderIndices *match;

    if (hmp->headers[id].start > 0)
    {
        *start = hmp->headers[id].start;
        *end = hmp->headers[id].end;
        return 1;
    }

    if (hmp->searched)
        return 0;

    if (pHttpConfig->header_matcher)
    {
        _dpd.searchAPI->search_instance_find_all(pHttpConfig->header_matcher,
                   (char *)data,
                   size, 0,
                   &http_header_pattern_match, (void*)hmp);
    }

    hmp->searched = 1;

    /*Close out search space for last matched if needed */
    if (hmp->last_match > 0 && hmp->headers[hmp->last_match].end <= 0)
        hmp->headers[hmp->last_match].end = size;

    match = &(hmp->headers[id]);
    if (match->start > 0)
    {
        *start = match->start;
        *end = match->end;
        return 1;
    }

    return 0;
}

tAppId getAppIdFromUrl(char *host, char *url, char **version,
                       char *referer, tAppId *clientAppId, tAppId *serviceAppId,
                       tAppId *payloadAppId, tAppId *referredPayloadAppId, unsigned from_rtmp,
                       const tDetectorHttpConfig *pHttpConfig)
{
    char *path;
    char *referer_start;
    char *temp_host = NULL;
    const char *referer_path = NULL;
    int host_len;
    int referer_len = 0;
    int referer_path_len = 0;
    int path_len = 0;
    tMlmpPattern patterns[3];
    tMlpPattern query;
    HostUrlDetectorPattern *data;
    char *q;
    int payload_found = 0;
    int url_len;
    static void *matcher;

#define RTMP_MEDIA_STREAM_OFFSET    50000000
#define URL_SCHEME_END_PATTERN "://"
#define URL_SCHEME_MAX_LEN     (sizeof("https://")-1)

    matcher = (from_rtmp ? pHttpConfig->RTMPHostUrlMatcher : pHttpConfig->hostUrlMatcher);

    if (!host && !url)
        return 0;

    if (url)
    {
        int scheme_len = strlen(url);
        if (scheme_len > URL_SCHEME_MAX_LEN)
            scheme_len = URL_SCHEME_MAX_LEN;    // only need to search the first few bytes for scheme
        char *url_offset = (char *)service_strstr((uint8_t *)url, scheme_len, (uint8_t *)URL_SCHEME_END_PATTERN, sizeof(URL_SCHEME_END_PATTERN)-1);
        if (url_offset)
            url_offset += sizeof(URL_SCHEME_END_PATTERN)-1;
        else
            return 0;

        url = url_offset;
        url_len = strlen(url);

    }
    else
    {
        url_len = 0;
    }

    if (!host)
    {
        host_len = url_len;

        temp_host = host = strdup(url);
        if (!temp_host)
        {
            return 0;
        }

        host  = strchr(host, '/');
        if (host != NULL)
        {
            *host = '\0';
        }
        host = temp_host;
    }
    host_len = strlen(host);

    if (url_len)
    {
        if (url_len < host_len)
        {
            free(temp_host);
            return 0;
        }

        path = strchr(url, '/');
        if (path)
            path_len = url + url_len - path;
    }

    if (!path_len)
    {
        path = "/";
        path_len = 1;
    }

    patterns[0].pattern = (uint8_t *) host;
    patterns[0].patternSize = host_len;
    patterns[1].pattern = (uint8_t *) path;
    patterns[1].patternSize = path_len;
    patterns[2].pattern = NULL;

    data = (HostUrlDetectorPattern *)mlmpMatchPatternUrl(matcher, patterns);

    if (data)
    {
        payload_found = 1;
        if (url)
        {
            q = strchr(url, '?');
            if (q != NULL)
            {
                char temp_ver[MAX_VERSION_SIZE];
                temp_ver[0] = 0;
                query.pattern = (uint8_t *) ++q;
                query.patternSize = strlen(q);

                matchQueryElements(&query, &data->query, temp_ver, MAX_VERSION_SIZE);

                if (temp_ver[0] != 0)
                {
                    optionallyReplaceWithStrdup(version,temp_ver);
                }
            }
        }

        *clientAppId = data->client_id;
        *serviceAppId = data->service_id;
        *payloadAppId = data->payload_id;
    }

    free(temp_host);

    /* if referred_id feature id disabled, referer will be null */
    if (referer && (!payload_found || appInfoEntryFlagGet(data->payload_id, APPINFO_FLAG_REFERRED, appIdActiveConfigGet())))
    {
        referer_start = referer;

        char *referer_offset = (char *)service_strstr((uint8_t *)referer_start, URL_SCHEME_MAX_LEN, (uint8_t *)URL_SCHEME_END_PATTERN, sizeof(URL_SCHEME_END_PATTERN)-1);
        if (referer_offset)
        {
            referer_offset += sizeof(URL_SCHEME_END_PATTERN)-1;
        }
        else
            return 0;

        referer_start = referer_offset;
        referer_len = strlen(referer_start);
        referer_path = strchr(referer_start, '/');

        if (referer_path)
        {
            referer_path_len = strlen(referer_path);
            referer_len -= referer_path_len;
        }
        else
        {
            referer_path = "/";
            referer_path_len = 1;
        }

        if (referer_start && referer_len > 0)
        {
            data = NULL;
            patterns[0].pattern = (uint8_t *)referer_start;
            patterns[0].patternSize = referer_len;
            patterns[1].pattern = (uint8_t *)referer_path;
            patterns[1].patternSize = referer_path_len;
            patterns[2].pattern = NULL;
            data = (HostUrlDetectorPattern *)mlmpMatchPatternUrl(matcher, patterns);
            if (data != NULL)
            {
                if (payload_found)
                    *referredPayloadAppId = *payloadAppId;
                else
                    payload_found = 1;
                *payloadAppId = data->payload_id;
            }
        }
    }
    return payload_found;
}

void getServerVendorVersion(const uint8_t *data, int len, char **version, char **vendor, RNAServiceSubtype **subtype)
{
    const uint8_t *subname;
    const uint8_t *subver;
    int subname_len;
    int subver_len;
    const uint8_t *paren;
    const uint8_t *ver;
    const uint8_t *p;
    const uint8_t *end = data + len;
    RNAServiceSubtype *sub;
    int vendor_len;
    int version_len;
    char *tmp;

    ver = memchr(data, '/', len);
    if (ver)
    {
        version_len = 0;
        vendor_len = ver - data;
        ver++;
        subname = NULL;
        subname_len = 0;
        subver = NULL;
        paren = NULL;
        for (p=ver; *p && p < end; p++)
        {
            if (*p == '(')
            {
                subname = NULL;
                paren = p;
            }
            else if (*p == ')')
            {
                subname = NULL;
                paren = NULL;
            }
            /* some admins put tags in their http response lines.
               the anchors will cause problems for adaptive profiles in snort,
               so let's just get rid of them */
            else if (*p == '<')
                break;
            else if (!paren)
            {
                if (*p == ' ' || *p == '\t')
                {
                    if (subname && subname_len > 0 && subver && *subname)
                    {
                        sub = calloc(1, sizeof(*sub));
                        if (sub)
                        {
                            if ((tmp = malloc(subname_len + 1)))
                            {
                                memcpy(tmp, subname, subname_len);
                                tmp[subname_len] = 0;
                                sub->service = tmp;
                            }
                            else
                            {
                                 _dpd.errMsg("getServerVendorVersion: "
                                         "Failed to allocate memory for service in sub\n");
                            }
                            subver_len = p - subver;
                            if (subver_len > 0 && *subver)
                            {
                                if ((tmp = malloc(subver_len + 1)))
                                {
                                    memcpy(tmp, subver, subver_len);
                                    tmp[subver_len] = 0;
                                    sub->version = tmp;
                                }
                                else
                                {
                                     _dpd.errMsg("getServerVendorVersion: "
                                             "Failed to allocate memory for version in sub\n");
                                }
                            }
                            sub->next = *subtype;
                            *subtype = sub;
                        }
                    }
                    subname = p + 1;
                    subname_len = 0;
                    subver = NULL;
                }
                else if (*p == '/' && subname)
                {
                    if (version_len <= 0)
                        version_len = subname - ver - 1;
                    subname_len = p - subname;
                    subver = p + 1;
                }
            }
        }
        if (subname && subname_len > 0 && subver && *subname)
        {
            sub = calloc(1, sizeof(*sub));
            if (sub)
            {
                if ((tmp = malloc(subname_len + 1)))
                {
                    memcpy(tmp, subname, subname_len);
                    tmp[subname_len] = 0;
                    sub->service = tmp;
                }
                else
                {
                     _dpd.errMsg("getServerVendorVersion: "
                             "Failed to allocate memory for service in sub\n");
                }
                subver_len = p - subver;
                if (subver_len > 0 && *subver)
                {
                    if ((tmp = malloc(subver_len + 1)))
                    {
                        memcpy(tmp, subver, subver_len);
                        tmp[subver_len] = 0;
                        sub->version = tmp;
                    }
                    else
                    {
                         _dpd.errMsg("getServerVendorVersion: "
                                 "Failed to allocate memory for version in sub\n");
                    }
                }
                sub->next = *subtype;
                *subtype = sub;
            }
        }
        if (version_len <= 0)
            version_len = p - ver;
        if (version_len >= MAX_VERSION_SIZE)
            version_len = MAX_VERSION_SIZE - 1;
        if (NULL != (*version = (char *)malloc(sizeof(char)*(version_len+1))))
        {
            memcpy(*version, ver, version_len);
            *(*version+version_len) = '\0';
        }
    }
    else
    {
        vendor_len = len;
    }

    if (vendor_len >= MAX_VERSION_SIZE)
        vendor_len = MAX_VERSION_SIZE - 1;
    if (NULL != (*vendor = (char *)malloc(sizeof(char)*(vendor_len+1))))
    {
        memcpy(*vendor, data, vendor_len);
        *(*vendor+vendor_len) = '\0';
    }
}

int webdav_found(HeaderMatchedPatterns *hmp)
{
    // to check for webdav, look for one of the special methods
    int found = 0;
    if (hmp->headers[HTTP_ID_COPY].start > 0)
        found = 1;
    else if (hmp->headers[HTTP_ID_MOVE].start > 0)
        found = 1;
    else if (hmp->headers[HTTP_ID_LOCK].start > 0)
        found = 1;
    else if (hmp->headers[HTTP_ID_UNLOCK].start > 0)
        found = 1;
    else if (hmp->headers[HTTP_ID_MKCOL].start > 0)
        found = 1;
    else if (hmp->headers[HTTP_ID_PROPPATCH].start > 0)
        found = 1;
    else if (hmp->headers[HTTP_ID_PROPFIND].start > 0)
        found = 1;
    return found;
}

// Start of HTTP/2 detection logic.
//
// This is intended to simply detect the presence of HTTP version 2 as a
// service protocol if it is seen (unencrypted) on non-std ports.  That way, we
// can notify Snort for future reference.  this covers the "with prior
// knowledge" case for HTTP/2 (i.e., the client knows the server supports
// HTTP/2 and jumps right in with the preface).

static CLIENT_APP_RETCODE http_client_init(const InitClientAppAPI * const init_api, SF_LIST *config);
static CLIENT_APP_RETCODE http_client_validate(const uint8_t *data, uint16_t size, const int dir,
                                               tAppIdData *flowp, SFSnortPacket *pkt, struct _Detector *userData,
                                               const tAppIdConfig *pConfig);

static int http_service_init(const InitServiceAPI * const init_api);
static int http_service_validate(ServiceValidationArgs* args);

static tAppRegistryEntry appIdRegistry[] =
{
    {APP_ID_HTTP, 0}
};

static const char HTTP2_PREFACE[] = "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n";
#define HTTP2_PREFACE_LEN (sizeof(HTTP2_PREFACE)-1)
#define HTTP2_PREFACE_MAXPOS (sizeof(HTTP2_PREFACE)-2)

typedef struct {
    const u_int8_t *pattern;
    unsigned length;
    int index;
    unsigned appId;
} Client_App_Pattern;

static Client_App_Pattern patterns[] =
{
    {(const uint8_t *)HTTP2_PREFACE, sizeof(HTTP2_PREFACE)-1, 0, APP_ID_HTTP},
};

SF_SO_PUBLIC tRNAClientAppModule http_client_mod =
{
    .name = "HTTP",
    .proto = IPPROTO_TCP,
    .init = &http_client_init,
    .validate = &http_client_validate,
    .minimum_matches = 1
};

static RNAServiceValidationPort pp[] =
{
    {NULL, 0, 0}
};

static tRNAServiceElement http_service_element =
{
    .next = NULL,
    .validate = &http_service_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "http",
    .ref_count = 1,
    .current_ref_count = 1,
};

tRNAServiceValidationModule http_service_mod =
{
    "HTTP",
    &http_service_init,
    pp
};

static CLIENT_APP_RETCODE http_client_init(const InitClientAppAPI * const init_api, SF_LIST *config)
{
    unsigned i;

    if (appidStaticConfig->http2_detection_enabled)
    {
        for (i=0; i < sizeof(patterns)/sizeof(*patterns); i++)
        {
            _dpd.debugMsg(DEBUG_LOG, "registering patterns: %s: %d",
                          (const char *)patterns[i].pattern, patterns[i].index);
            init_api->RegisterPattern(&http_client_validate, IPPROTO_TCP, patterns[i].pattern, patterns[i].length, patterns[i].index, init_api->pAppidConfig);
        }
    }


    unsigned j;
    for (j=0; j < sizeof(appIdRegistry)/sizeof(*appIdRegistry); j++)
    {
        _dpd.debugMsg(DEBUG_LOG, "registering appId: %d\n", appIdRegistry[j].appId);
        init_api->RegisterAppId(&http_client_validate, appIdRegistry[j].appId, appIdRegistry[j].additionalInfo, init_api->pAppidConfig);
    }

    return CLIENT_APP_SUCCESS;
}

static CLIENT_APP_RETCODE http_client_validate(const uint8_t *data, uint16_t size, const int dir,
                                               tAppIdData *flowp, SFSnortPacket *pkt, struct _Detector *userData,
                                               const tAppIdConfig *pConfig)
{
    http_client_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_HTTP, APP_ID_HTTP + GENERIC_APP_OFFSET, NULL);
    flowp->rnaClientState = RNA_STATE_FINISHED;
    http_service_mod.api->add_service(flowp, pkt, dir, &http_service_element,
                                      APP_ID_HTTP, NULL, NULL, NULL, NULL);
    flowp->rnaServiceState = RNA_STATE_FINISHED;
    setAppIdFlag(flowp, APPID_SESSION_CLIENT_DETECTED | APPID_SESSION_SERVICE_DETECTED);
    clearAppIdFlag(flowp, APPID_SESSION_CONTINUE);
    flowp->is_http2 = true;

    return CLIENT_APP_SUCCESS;
}

static int http_service_init(const InitServiceAPI * const init_api)
{
    unsigned i;
    for (i=0; i < sizeof(appIdRegistry)/sizeof(*appIdRegistry); i++)
    {
        _dpd.debugMsg(DEBUG_LOG, "registering appId: %d\n", appIdRegistry[i].appId);
        init_api->RegisterAppId(&http_service_validate, appIdRegistry[i].appId, appIdRegistry[i].additionalInfo, init_api->pAppidConfig);
    }

    return 0;
}

static int http_service_validate(ServiceValidationArgs* args)
{
    return SERVICE_INPROCESS;
}

// End of HTTP/2 detection logic.
