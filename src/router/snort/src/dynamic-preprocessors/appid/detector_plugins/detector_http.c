/*
** Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
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
#include "detector_api.h"
#include "httpCommon.h"
#include "http_url_patterns.h"
#include "detector_http.h"
#include "fw_appid.h"

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
#if 0
static const char MOZILLA_PATTERN[] = "Mozilla/";
static const char COMPATIBLE_PATTERN[] = "compatible;";
#endif
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

/* bitTorrent url patterns */
static const char ANNOUNCE[] = "/announce?";
static const char INFO_HASH[] = "info_hash";
static const char PEER_ID[] = "peer_id";
static const char PORT[] = "port";
static const char PATTERN_IP[] = "ip";
static const char UPLOADED[] = "uploaded";
static const char DOWNLOADED[] = "downloaded";
static const char LEFT[] = "left";
static const char EVENT[] = "event";
static const char COMPACT[] = "compact";
static const char KEY[] = "key";
static const char TRACKERID[] = "trackerid";
static const char NUMWANT[] = "numwant";
#define ANNOUNCE_SIZE (sizeof(ANNOUNCE)-1)
#define INFO_HASH_SIZE (sizeof(INFO_HASH)-1)
#define PEER_ID_SIZE (sizeof(PEER_ID)-1)
#define PORT_SIZE (sizeof(PORT)-1)
#define PATTERN_IP_SIZE (sizeof(PATTERN_IP)-1)
#define UPLOADED_SIZE (sizeof(UPLOADED)-1)
#define DOWNLOADED_SIZE (sizeof(DOWNLOADED)-1)
#define LEFT_SIZE (sizeof(LEFT)-1)
#define EVENT_SIZE (sizeof(EVENT)-1)
#define COMPACT_SIZE (sizeof(COMPACT)-1)
#define KEY_SIZE (sizeof(KEY)-1)
#define TRACKERID_SIZE (sizeof(TRACKERID)-1)
#define NUMWANT_SIZE (sizeof(NUMWANT)-1)

/* skype pattern */
static const char SKYPE_HOST[] = "ui.skype.com";
static const char SKYPE_VER[] = "ver=";
#define SKYPE_HOST_SIZE (sizeof(SKYPE_HOST)-1)
#define SKYPE_VER_SIZE (sizeof(SKYPE_VER)-1)

#define COMPATIBLE_BROWSER_STRING " (Compat)"

typedef struct _MatchedPatterns {
    DetectorHTTPPattern *mpattern;
    int index;
    struct _MatchedPatterns *next;
} MatchedPatterns;

static void *url_matcher;
static void *client_agent_matcher;
static void *via_matcher;
static void *hostUrlMatcher;
static void *RTMPHostUrlMatcher;
static void *header_matcher;
static void *content_type_matcher;

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
    { SINGLE, APP_ID_SKYPE_AUTH, APP_ID_SKYPE, 0,
      SKYPE_HOST_SIZE, (u_int8_t *)SKYPE_HOST, APP_ID_SKYPE_AUTH,
    },
    { SINGLE, 0, 0, APP_ID_GOOGLE_TOOLBAR,
      GOOGLE_TB_PATTERN_SIZE, (u_int8_t *)GOOGLE_TB_PATTERN, APP_ID_GOOGLE_TOOLBAR,
    },
};

static DetectorHTTPPattern url_patterns[] =
{
    { SKYPE_URL, APP_ID_SKYPE_AUTH, APP_ID_SKYPE, 0,
      SKYPE_HOST_SIZE, (u_int8_t *)SKYPE_HOST, APP_ID_SKYPE_AUTH,
    },
    { SKYPE_VERSION, APP_ID_SKYPE_AUTH, APP_ID_SKYPE, 0,
      SKYPE_VER_SIZE, (u_int8_t *)SKYPE_VER, APP_ID_SKYPE_AUTH,
    },
    { BT_ANNOUNCE, APP_ID_BITTORRENT, APP_ID_BITTORRENT, 0,
      ANNOUNCE_SIZE, (u_int8_t *)ANNOUNCE, APP_ID_BITTORRENT,
    },
    { BT_OTHER, APP_ID_BITTORRENT, APP_ID_BITTORRENT, 0,
      INFO_HASH_SIZE, (u_int8_t *)INFO_HASH, APP_ID_BITTORRENT,
    },
    { BT_OTHER, APP_ID_BITTORRENT, APP_ID_BITTORRENT, 0,
      PEER_ID_SIZE, (u_int8_t *)PEER_ID, APP_ID_BITTORRENT,
    },
    { BT_OTHER, APP_ID_BITTORRENT, APP_ID_BITTORRENT, 0,
      PORT_SIZE, (u_int8_t *)PORT, APP_ID_BITTORRENT,
    },
    { BT_OTHER, APP_ID_BITTORRENT, APP_ID_BITTORRENT, 0,
      PATTERN_IP_SIZE, (u_int8_t *)PATTERN_IP, APP_ID_BITTORRENT,
    },
    { BT_OTHER, APP_ID_BITTORRENT, APP_ID_BITTORRENT, 0,
      UPLOADED_SIZE, (u_int8_t *)UPLOADED, APP_ID_BITTORRENT,
    },
    { BT_OTHER, APP_ID_BITTORRENT, APP_ID_BITTORRENT, 0,
      DOWNLOADED_SIZE, (u_int8_t *)DOWNLOADED, APP_ID_BITTORRENT,
    },
    { BT_OTHER, APP_ID_BITTORRENT, APP_ID_BITTORRENT, 0,
      LEFT_SIZE, (u_int8_t *)LEFT, APP_ID_BITTORRENT,
    },
    { BT_OTHER, APP_ID_BITTORRENT, APP_ID_BITTORRENT, 0,
      EVENT_SIZE, (u_int8_t *)EVENT, APP_ID_BITTORRENT,
    },
    { BT_OTHER, APP_ID_BITTORRENT, APP_ID_BITTORRENT, 0,
      COMPACT_SIZE, (u_int8_t *)COMPACT, APP_ID_BITTORRENT,
    },
    { BT_OTHER, APP_ID_BITTORRENT, APP_ID_BITTORRENT, 0,
      KEY_SIZE, (u_int8_t *)KEY, APP_ID_BITTORRENT,
    },
    { BT_OTHER, APP_ID_BITTORRENT, APP_ID_BITTORRENT, 0,
      TRACKERID_SIZE, (u_int8_t *)TRACKERID, APP_ID_BITTORRENT,
    },
    { BT_OTHER, APP_ID_BITTORRENT, APP_ID_BITTORRENT, 0,
      NUMWANT_SIZE, (u_int8_t *)NUMWANT, APP_ID_BITTORRENT,
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
#if 1 /*Commented out because we are not using any of these Headers yet */
static const char HTTP_HEADER_ACCEPT[] = "Accept:";
static const char HTTP_HEADER_ACCEPT_CHARSET[] = "Accept-Charset:";
static const char HTTP_HEADER_ACCEPT_ENCODING[] = "Accept-Encoding:";
static const char HTTP_HEADER_ACCEPT_LANGUAGE[] = "Accept-Language:";
static const char HTTP_HEADER_ACCEPT_RANGES[] = "Accept-Ranges:";
static const char HTTP_HEADER_AGE[] = "Age:";
static const char HTTP_HEADER_ALLOW[] = "Allow:";
static const char HTTP_HEADER_AUTHORIZATION[] = "Authorization:";
static const char HTTP_HEADER_CACHE_CONTROL[] = "Cache-Control:";
static const char HTTP_HEADER_CONNECTION[] = "Connection:";
static const char HTTP_HEADER_COOKIE[] = "Cookie:";
static const char HTTP_HEADER_CONTENT_DISPOSITION[] = "Content-Dispisition:";
static const char HTTP_HEADER_CONTENT_ENCODING[] = "Content-Encoding:";
static const char HTTP_HEADER_CONTENT_LANGUAGE[] = "Content-Language:";
static const char HTTP_HEADER_CONTENT_LENGTH[] = "Content-Length:";
static const char HTTP_HEADER_CONTENT_LOCATION[] = "Content-Location:";
static const char HTTP_HEADER_CONTENT_MD5[] = "Content-MD5:";
static const char HTTP_HEADER_CONTENT_RANGE[] = "Content-Range:";
static const char HTTP_HEADER_DATE[] = "Date:";
static const char HTTP_HEADER_ETAG[] = "Etag:";
static const char HTTP_HEADER_EXPECT[] = "Expect:";
static const char HTTP_HEADER_EXPIRES[] = "Expires:";
static const char HTTP_HEADER_FROM[] = "From:";
static const char HTTP_HEADER_HOST[] = "Host:";
static const char HTTP_HEADER_IF_MATCH[] = "If-Match:";
static const char HTTP_HEADER_IF_MODIFIED_SINCE[] = "If-Modified:";
static const char HTTP_HEADER_IF_NONE_MATCH[] = "If-None-Match:";
static const char HTTP_HEADER_IF_RANGE[] = "If-Range:";
static const char HTTP_HEADER_IF_UNMODIFIED_SINCE[] = "If-Unmodified-Since:";
static const char HTTP_HEADER_LAST_MODIFIED[] = "Last-Modified:";
static const char HTTP_HEADER_LINK[] = "Link:";
static const char HTTP_HEADER_LOCATION[] = "Location:";
static const char HTTP_HEADER_MAX_FORWARDS[] = "Max-Forwards:";
static const char HTTP_HEADER_P3P[] = "P3P:";
static const char HTTP_HEADER_PRAGMA[] = "Pragma:";
static const char HTTP_HEADER_PROXY_AUTHORIZATION[] = "Proxy-Authorization:";
static const char HTTP_HEADER_PROXY_AUTHENTICATE[] = "Proxy-Authenticate:";
static const char HTTP_HEADER_RANGE[] = "Range:";
static const char HTTP_HEADER_REFERER[] = "Referer:";
static const char HTTP_HEADER_REFRESH[] = "Refresh:";
static const char HTTP_HEADER_RETRY_AFTER[] = "Retry-After:";
static const char HTTP_HEADER_SET_COOKIE[] = "Set-Cookie:";
static const char HTTP_HEADER_STRICT_TRANSPORT_SECURITY[] = "Strict-Transport-Security:";
static const char HTTP_HEADER_TE[] = "TE:";
static const char HTTP_HEADER_TRAILER[] = "Trailer:";
static const char HTTP_HEADER_TRANSFER_ENCODING[] = "Transfer-Encoding:";
static const char HTTP_HEADER_UPGRADE[] = "Upgrade:";
static const char HTTP_HEADER_USER_AGENT[] = "User-Agent: ";
static const char HTTP_HEADER_VARY[] = "Vary:";
static const char HTTP_HEADER_VIA_FIELD[] = "Via: ";
static const char HTTP_HEADER_WARNING[] = "Warning:";
static const char HTTP_HEADER_WWW_AUTHENTICATE[] = "WWW-Authenticate:";
#endif

#define HTTP_HEADER_CONTENT_TYPE_SIZE (sizeof(HTTP_HEADER_CONTENT_TYPE)-1)
#define HTTP_HEADER_SERVER_FIELD_SIZE (sizeof(HTTP_HEADER_SERVER_FIELD)-1)
#define HTTP_HEADER_X_WORKING_WITH_SIZE (sizeof(HTTP_HEADER_X_WORKING_WITH)-1)
#define HTTP_HEADER_CRLF_SIZE (sizeof(HTTP_HEADER_CRLF)-1)
#define HTTP_HEADER_LF_SIZE (sizeof(HTTP_HEADER_LF)-1)
#if 1 /* Comment out since we don't use these headers yet */
#define HTTP_HEADER_ACCEPT_SIZE (sizeof(HTTP_HEADER_ACCEPT)-1)
#define HTTP_HEADER_ACCEPT_CHARSET_SIZE (sizeof(HTTP_HEADER_ACCEPT_CHARSET)-1)
#define HTTP_HEADER_ACCEPT_ENCODING_SIZE (sizeof(HTTP_HEADER_ACCEPT_ENCODING)-1)
#define HTTP_HEADER_ACCEPT_LANGUAGE_SIZE (sizeof(HTTP_HEADER_ACCEPT_LANGUAGE)-1)
#define HTTP_HEADER_ACCEPT_RANGES_SIZE (sizeof(HTTP_HEADER_ACCEPT_RANGES)-1)
#define HTTP_HEADER_AGE_SIZE (sizeof(HTTP_HEADER_AGE)-1)
#define HTTP_HEADER_ALLOW_SIZE (sizeof(HTTP_HEADER_ALLOW)-1)
#define HTTP_HEADER_AUTHORIZATION_SIZE (sizeof(HTTP_HEADER_AUTHORIZATION)-1)
#define HTTP_HEADER_CACHE_CONTROL_SIZE (sizeof(HTTP_HEADER_CACHE_CONTROL)-1)
#define HTTP_HEADER_CONNECTION_SIZE (sizeof(HTTP_HEADER_CONNECTION)-1)
#define HTTP_HEADER_COOKIE_SIZE (sizeof(HTTP_HEADER_COOKIE)-1)
#define HTTP_HEADER_CONTENT_DISPOSITION_SIZE (sizeof(HTTP_HEADER_CONTENT_DISPOSITION)-1)
#define HTTP_HEADER_CONTENT_ENCODING_SIZE (sizeof(HTTP_HEADER_CONTENT_ENCODING)-1)
#define HTTP_HEADER_CONTENT_LANGUAGE_SIZE (sizeof(HTTP_HEADER_CONTENT_LANGUAGE)-1)
#define HTTP_HEADER_CONTENT_LENGTH_SIZE (sizeof(HTTP_HEADER_CONTENT_LENGTH)-1)
#define HTTP_HEADER_CONTENT_LOCATION_SIZE (sizeof(HTTP_HEADER_CONTENT_LOCATION)-1)
#define HTTP_HEADER_CONTENT_MD5_SIZE (sizeof(HTTP_HEADER_CONTENT_MD5)-1)
#define HTTP_HEADER_CONTENT_RANGE_SIZE (sizeof(HTTP_HEADER_CONTENT_RANGE)-1)
#define HTTP_HEADER_DATE_SIZE (sizeof(HTTP_HEADER_DATE)-1)
#define HTTP_HEADER_ETAG_SIZE (sizeof(HTTP_HEADER_ETAG)-1)
#define HTTP_HEADER_EXPECT_SIZE (sizeof(HTTP_HEADER_EXPECT)-1)
#define HTTP_HEADER_EXPIRES_SIZE (sizeof(HTTP_HEADER_EXPIRES)-1)
#define HTTP_HEADER_FROM_SIZE (sizeof(HTTP_HEADER_FROM)-1)
#define HTTP_HEADER_HOST_SIZE (sizeof(HTTP_HEADER_HOST)-1)
#define HTTP_HEADER_IF_MATCH_SIZE (sizeof(HTTP_HEADER_IF_MATCH)-1)
#define HTTP_HEADER_IF_MODIFIED_SINCE_SIZE (sizeof(HTTP_HEADER_IF_MODIFIED_SINCE)-1)
#define HTTP_HEADER_IF_NONE_MATCH_SIZE (sizeof(HTTP_HEADER_IF_NONE_MATCH)-1)
#define HTTP_HEADER_IF_RANGE_SIZE (sizeof(HTTP_HEADER_IF_RANGE)-1)
#define HTTP_HEADER_IF_UNMODIFIED_SINCE_SIZE (sizeof(HTTP_HEADER_IF_UNMODIFIED_SINCE)-1)
#define HTTP_HEADER_LAST_MODIFIED_SIZE (sizeof(HTTP_HEADER_LAST_MODIFIED)-1)
#define HTTP_HEADER_LINK_SIZE (sizeof(HTTP_HEADER_LINK)-1)
#define HTTP_HEADER_LOCATION_SIZE (sizeof(HTTP_HEADER_LOCATION)-1)
#define HTTP_HEADER_MAX_FORWARDS_SIZE (sizeof(HTTP_HEADER_MAX_FORWARDS)-1)
#define HTTP_HEADER_P3P_SIZE (sizeof(HTTP_HEADER_P3P)-1)
#define HTTP_HEADER_PRAGMA_SIZE (sizeof(HTTP_HEADER_PRAGMA)-1)
#define HTTP_HEADER_PROXY_AUTHORIZATION_SIZE (sizeof(HTTP_HEADER_PROXY_AUTHORIZATION)-1)
#define HTTP_HEADER_PROXY_AUTHENTICATE_SIZE (sizeof(HTTP_HEADER_PROXY_AUTHENTICATE)-1)
#define HTTP_HEADER_RANGE_SIZE (sizeof(HTTP_HEADER_RANGE)-1)
#define HTTP_HEADER_REFERER_SIZE (sizeof(HTTP_HEADER_REFERER)-1)
#define HTTP_HEADER_REFRESH_SIZE (sizeof(HTTP_HEADER_REFRESH)-1)
#define HTTP_HEADER_RETRY_AFTER_SIZE (sizeof(HTTP_HEADER_RETRY_AFTER)-1)
#define HTTP_HEADER_SET_COOKIE_SIZE (sizeof(HTTP_HEADER_SET_COOKIE)-1)
#define HTTP_HEADER_STRICT_TRANSPORT_SECURITY_SIZE (sizeof(HTTP_HEADER_STRICT_TRANSPORT_SECURITY)-1)
#define HTTP_HEADER_TE_SIZE (sizeof(HTTP_HEADER_TE)-1)
#define HTTP_HEADER_TRAILER_SIZE (sizeof(HTTP_HEADER_TRAILER)-1)
#define HTTP_HEADER_TRANSFER_ENCODING_SIZE (sizeof(HTTP_HEADER_TRANSFER_ENCODING)-1)
#define HTTP_HEADER_UPGRADE_SIZE (sizeof(HTTP_HEADER_UPGRADE)-1)
#define HTTP_HEADER_USER_AGENT_SIZE (sizeof(HTTP_HEADER_USER_AGENT)-1)
#define HTTP_HEADER_VARY_SIZE (sizeof(HTTP_HEADER_VARY)-1)
#define HTTP_HEADER_VIA_FIELD_SIZE (sizeof(HTTP_HEADER_VIA_FIELD)-1)
#define HTTP_HEADER_WARNING_SIZE (sizeof(HTTP_HEADER_WARNING)-1)
#define HTTP_HEADER_WWW_AUTHENTICATE_SIZE (sizeof(HTTP_HEADER_WWW_AUTHENTICATE)-1)
#endif

static HeaderPattern header_patterns[] =
{
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
    {HTTP_ID_LEN, (uint8_t *) HTTP_HEADER_CRLF, HTTP_HEADER_CRLF_SIZE},
    {HTTP_ID_LEN, (uint8_t *) HTTP_HEADER_LF, HTTP_HEADER_LF_SIZE},
#if 0 /* Comment out until we need these headers */
    {HTTP_ID_CONTENT_TYPE, (uint8_t *) HTTP_HEADER_CONTENT_TYPE,HTTP_HEADER_CONTENT_TYPE_SIZE},
    {HTTP_ID_SERVER, (uint8_t *) HTTP_HEADER_SERVER_FIELD,HTTP_HEADER_SERVER_FIELD_SIZE},
    {HTTP_ID_X_WORKING_WITH, (uint8_t *) HTTP_HEADER_X_WORKING_WITH,HTTP_HEADER_X_WORKING_WITH_SIZE},
    {HTTP_ID_ACCEPT, (uint8_t *)HTTP_HEADER_ACCEPT, HTTP_HEADER_ACCEPT_SIZE},
    {HTTP_ID_ACCEPT_CHARSET, (uint8_t *)HTTP_HEADER_ACCEPT_CHARSET, HTTP_HEADER_ACCEPT_CHARSET_SIZE},
    {HTTP_ID_ACCEPT_ENCODING, (uint8_t *)HTTP_HEADER_ACCEPT_ENCODING, HTTP_HEADER_ACCEPT_ENCODING_SIZE},
    {HTTP_ID_ACCEPT_LANGUAGE, (uint8_t *)HTTP_HEADER_ACCEPT_LANGUAGE, HTTP_HEADER_ACCEPT_LANGUAGE_SIZE},
    {HTTP_ID_ACCEPT_RANGES, (uint8_t *)HTTP_HEADER_ACCEPT_RANGES, HTTP_HEADER_ACCEPT_RANGES_SIZE},
    {HTTP_ID_AGE, (uint8_t *)HTTP_HEADER_AGE, HTTP_HEADER_AGE_SIZE},
    {HTTP_ID_ALLOW, (uint8_t *)HTTP_HEADER_ALLOW, HTTP_HEADER_ALLOW_SIZE},
    {HTTP_ID_AUTHORIZATION, (uint8_t *)HTTP_HEADER_AUTHORIZATION, HTTP_HEADER_AUTHORIZATION_SIZE},
    {HTTP_ID_CACHE_CONTROL, (uint8_t *)HTTP_HEADER_CACHE_CONTROL, HTTP_HEADER_CACHE_CONTROL_SIZE},
    {HTTP_ID_CONNECTION, (uint8_t *)HTTP_HEADER_CONNECTION, HTTP_HEADER_CONNECTION_SIZE},
    {HTTP_ID_COOKIE, (uint8_t *)HTTP_HEADER_COOKIE, HTTP_HEADER_COOKIE_SIZE},
    {HTTP_ID_CONTENT_DISPOSITION, (uint8_t *) HTTP_HEADER_CONTENT_DISPOSITION,HTTP_HEADER_CONTENT_DISPOSITION_SIZE},
    {HTTP_ID_CONTENT_ENCODING, (uint8_t *) HTTP_HEADER_CONTENT_ENCODING,HTTP_HEADER_CONTENT_ENCODING_SIZE},
    {HTTP_ID_CONTENT_LANGUAGE, (uint8_t *) HTTP_HEADER_CONTENT_LANGUAGE,HTTP_HEADER_CONTENT_LANGUAGE_SIZE},
    {HTTP_ID_CONTENT_LENGTH, (uint8_t *) HTTP_HEADER_CONTENT_LENGTH,HTTP_HEADER_CONTENT_LENGTH_SIZE},
    {HTTP_ID_CONTENT_LOCATION, (uint8_t *) HTTP_HEADER_CONTENT_LOCATION,HTTP_HEADER_CONTENT_LOCATION_SIZE},
    {HTTP_ID_CONTENT_MD5, (uint8_t *) HTTP_HEADER_CONTENT_MD5,HTTP_HEADER_CONTENT_MD5_SIZE},
    {HTTP_ID_CONTENT_RANGE, (uint8_t *) HTTP_HEADER_CONTENT_RANGE,HTTP_HEADER_CONTENT_RANGE_SIZE},
    {HTTP_ID_DATE, (uint8_t *) HTTP_HEADER_DATE,HTTP_HEADER_DATE_SIZE},
    {HTTP_ID_ETAG, (uint8_t *) HTTP_HEADER_ETAG,HTTP_HEADER_ETAG_SIZE},
    {HTTP_ID_EXPECT, (uint8_t *) HTTP_HEADER_EXPECT,HTTP_HEADER_EXPECT_SIZE},
    {HTTP_ID_EXPIRES, (uint8_t *) HTTP_HEADER_EXPIRES,HTTP_HEADER_EXPIRES_SIZE},
    {HTTP_ID_FROM, (uint8_t *) HTTP_HEADER_FROM,HTTP_HEADER_FROM_SIZE},
    {HTTP_ID_HOST, (uint8_t *) HTTP_HEADER_HOST,HTTP_HEADER_HOST_SIZE},
    {HTTP_ID_IF_MATCH, (uint8_t *) HTTP_HEADER_IF_MATCH,HTTP_HEADER_IF_MATCH_SIZE},
    {HTTP_ID_IF_MODIFIED_SINCE, (uint8_t *) HTTP_HEADER_IF_MODIFIED_SINCE,HTTP_HEADER_IF_MODIFIED_SINCE_SIZE},
    {HTTP_ID_IF_NONE_MATCH, (uint8_t *) HTTP_HEADER_IF_NONE_MATCH,HTTP_HEADER_IF_NONE_MATCH_SIZE},
    {HTTP_ID_IF_RANGE, (uint8_t *) HTTP_HEADER_IF_RANGE,HTTP_HEADER_IF_RANGE_SIZE},
    {HTTP_ID_IF_UNMODIFIED_SINCE, (uint8_t *) HTTP_HEADER_IF_UNMODIFIED_SINCE,HTTP_HEADER_IF_UNMODIFIED_SINCE_SIZE},
    {HTTP_ID_LAST_MODIFIED, (uint8_t *) HTTP_HEADER_LAST_MODIFIED,HTTP_HEADER_LAST_MODIFIED_SIZE},
    {HTTP_ID_LINK, (uint8_t *) HTTP_HEADER_LINK,HTTP_HEADER_LINK_SIZE},
    {HTTP_ID_LOCATION, (uint8_t *) HTTP_HEADER_LOCATION,HTTP_HEADER_LOCATION_SIZE},
    {HTTP_ID_MAX_FORWARDS, (uint8_t *) HTTP_HEADER_MAX_FORWARDS,HTTP_HEADER_MAX_FORWARDS_SIZE},
    {HTTP_ID_P3P, (uint8_t *) HTTP_HEADER_P3P,HTTP_HEADER_P3P_SIZE},
    {HTTP_ID_PRAGMA, (uint8_t *) HTTP_HEADER_PRAGMA,HTTP_HEADER_PRAGMA_SIZE},
    {HTTP_ID_PROXY_AUTHORIZATION, (uint8_t *) HTTP_HEADER_PROXY_AUTHORIZATION,HTTP_HEADER_PROXY_AUTHORIZATION_SIZE},
    {HTTP_ID_PROXY_AUTHENTICATE, (uint8_t *) HTTP_HEADER_PROXY_AUTHENTICATE,HTTP_HEADER_PROXY_AUTHENTICATE_SIZE},
    {HTTP_ID_RANGE, (uint8_t *) HTTP_HEADER_RANGE,HTTP_HEADER_RANGE_SIZE},
    {HTTP_ID_REFERER, (uint8_t *) HTTP_HEADER_REFERER,HTTP_HEADER_REFERER_SIZE},
    {HTTP_ID_REFRESH, (uint8_t *) HTTP_HEADER_REFRESH,HTTP_HEADER_REFRESH_SIZE},
    {HTTP_ID_RETRY_AFTER, (uint8_t *) HTTP_HEADER_RETRY_AFTER,HTTP_HEADER_RETRY_AFTER_SIZE},
    {HTTP_ID_SET_COOKIE, (uint8_t *) HTTP_HEADER_SET_COOKIE,HTTP_HEADER_SET_COOKIE_SIZE},
    {HTTP_ID_STRICT_TRANSPORT_SECURITY, (uint8_t *) HTTP_HEADER_STRICT_TRANSPORT_SECURITY,HTTP_HEADER_STRICT_TRANSPORT_SECURITY_SIZE},
    {HTTP_ID_TE, (uint8_t *) HTTP_HEADER_TE,HTTP_HEADER_TE_SIZE},
    {HTTP_ID_TRAILER, (uint8_t *) HTTP_HEADER_TRAILER,HTTP_HEADER_TRAILER_SIZE},
    {HTTP_ID_TRANSFER_ENCODING, (uint8_t *) HTTP_HEADER_TRANSFER_ENCODING,HTTP_HEADER_TRANSFER_ENCODING_SIZE},
    {HTTP_ID_UPGRADE, (uint8_t *) HTTP_HEADER_UPGRADE,HTTP_HEADER_UPGRADE_SIZE},
    {HTTP_ID_USER_AGENT, (uint8_t *) HTTP_HEADER_USER_AGENT,HTTP_HEADER_USER_AGENT_SIZE},
    {HTTP_ID_VARY, (uint8_t *) HTTP_HEADER_VARY,HTTP_HEADER_VARY_SIZE},
    {HTTP_ID_VIA, (uint8_t *) HTTP_HEADER_VIA_FIELD,HTTP_HEADER_VIA_FIELD_SIZE},
    {HTTP_ID_WARNING, (uint8_t *) HTTP_HEADER_WARNING,HTTP_HEADER_WARNING_SIZE},
    {HTTP_ID_WWW_AUTHENTICATE, (uint8_t *) HTTP_HEADER_WWW_AUTHENTICATE,HTTP_HEADER_WWW_AUTHENTICATE_SIZE}
#endif
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
        (*patternIndex)++;
    }

    /* Add patterns from Lua API */
    for(element = luaPatternList; element != 0; element = element->next)
    {
        _dpd.searchAPI->search_instance_add_ex(patternMatcher,
                (char *)element->detectorHTTPPattern.pattern,
                element->detectorHTTPPattern.pattern_size,
                &element->detectorHTTPPattern,
                STR_SEARCH_CASE_SENSITIVE);
        (*patternIndex)++;
    }

    _dpd.searchAPI->search_instance_prep(patternMatcher);

    return patternMatcher;
}

static int processHostPatterns(
        DetectorHTTPPattern *patternList,
        size_t patternListCount,
        HTTPListElement* luaPatternList,
        DetectorAppUrlList *urlPatternList,
        DetectorAppUrlList *RTMPUrlList
        )
{
    HTTPListElement* element;
    u_int32_t i;
    DetectorAppUrlPattern *appUrlPattern;

    if (!hostUrlMatcher)
        hostUrlMatcher = mlmpCreate();

    if (!RTMPHostUrlMatcher)
        RTMPHostUrlMatcher = mlmpCreate();

    for (i=0; i < patternListCount; i++)
    {
        if (addMlmpPattern(hostUrlMatcher, patternList[i].pattern, patternList[i].pattern_size,
                NULL, 0, NULL, 0, patternList[i].appId, patternList[i].payload, patternList[i].service_id,
                patternList[i].client_app, patternList[i].seq) < 0)
            return -1;
    }

    for(element = luaPatternList; element != 0; element = element->next)
    {
        if (addMlmpPattern(hostUrlMatcher, element->detectorHTTPPattern.pattern, element->detectorHTTPPattern.pattern_size,
                NULL, 0, NULL, 0, element->detectorHTTPPattern.appId, element->detectorHTTPPattern.payload, element->detectorHTTPPattern.service_id,
                element->detectorHTTPPattern.client_app, element->detectorHTTPPattern.seq) < 0)
            return -1;
    }

    for (i = 0; i < RTMPUrlList->usedCount; i++)
    {
        appUrlPattern = RTMPUrlList->urlPattern[i];
        if (addMlmpPattern(RTMPHostUrlMatcher, appUrlPattern->patterns.host.pattern, appUrlPattern->patterns.host.patternSize,
                appUrlPattern->patterns.path.pattern, appUrlPattern->patterns.path.patternSize,
                appUrlPattern->userData.query.pattern, appUrlPattern->userData.query.patternSize, appUrlPattern->userData.appId,
                appUrlPattern->userData.payload, appUrlPattern->userData.service_id, appUrlPattern->userData.client_app, SINGLE) < 0)
            return -1;
    }

    for (i = 0; i < urlPatternList->usedCount; i++)
    {
        appUrlPattern = urlPatternList->urlPattern[i];
        if (addMlmpPattern(hostUrlMatcher, appUrlPattern->patterns.host.pattern, appUrlPattern->patterns.host.patternSize,
                appUrlPattern->patterns.path.pattern, appUrlPattern->patterns.path.patternSize,
                appUrlPattern->userData.query.pattern, appUrlPattern->userData.query.patternSize, appUrlPattern->userData.appId,
                appUrlPattern->userData.payload, appUrlPattern->userData.service_id, appUrlPattern->userData.client_app, SINGLE) < 0)
            return -1;
    }

    mlmpProcessPatterns(hostUrlMatcher);
    mlmpProcessPatterns(RTMPHostUrlMatcher);
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
        (*patternIndex)++;
    }

    /* Add patterns from Lua API */
    for(element = luaPatternList; element; element = element->next)
    {
        _dpd.searchAPI->search_instance_add_ex(patternMatcher,
                (char  *)element->detectorHTTPPattern.pattern,
                element->detectorHTTPPattern.pattern_size,
                &element->detectorHTTPPattern,
                STR_SEARCH_CASE_SENSITIVE);
        (*patternIndex)++;
    }

    _dpd.searchAPI->search_instance_prep(patternMatcher);

    return patternMatcher;
}

static void* registerHeaderPatterns(
        HeaderPattern *patternList,
        size_t patternListCount,
        size_t *patternIndex)
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
        (*patternIndex)++;
    }

    _dpd.searchAPI->search_instance_prep(patternMatcher);

    return patternMatcher;
}

int http_detector_finalize(void)
{
    size_t upc = 0;
    size_t apc = 0;
    size_t hfc = 0;
    size_t ctc = 0;
    size_t vpc = 0;

    HttpPatternLists* patternLists = &httpPatternLists;
    u_int32_t numPatterns;
    /* Set up MPSE for proxies */
    if (via_matcher)
        _dpd.searchAPI->search_instance_free(via_matcher);

    /*create via pattern matcher */
    numPatterns = sizeof(via_http_detector_patterns)/sizeof(*via_http_detector_patterns);
    via_matcher = processPatterns(via_http_detector_patterns, numPatterns, &vpc, NULL);
    if (!via_matcher) return -1;

    /*create url pattern matcher */
    numPatterns = sizeof(url_patterns)/sizeof(*url_patterns);
    url_matcher = processPatterns(url_patterns, numPatterns, &upc,
                             patternLists->urlPatternList);
    if (!url_matcher) return -1;

    /*create client agent pattern matcher */
    numPatterns = sizeof(client_agent_patterns)/sizeof(*client_agent_patterns);
    client_agent_matcher = processPatterns(client_agent_patterns,numPatterns, &apc,
                               patternLists->clientAgentPatternList);
    if (!client_agent_matcher) return -1;

    numPatterns = sizeof(header_patterns)/sizeof(*header_patterns);
    header_matcher = registerHeaderPatterns(header_patterns,numPatterns, &hfc);
    if (!header_matcher) return -1;

    numPatterns = sizeof(host_payload_http_detector_patterns)/sizeof(*host_payload_http_detector_patterns);
    if (processHostPatterns(host_payload_http_detector_patterns, numPatterns, patternLists->hostPayloadPatternList, &patternLists->appUrlList, &patternLists->RTMPUrlList) < 0)
        return -1;

    numPatterns = sizeof(content_type_patterns)/sizeof(*content_type_patterns);
    content_type_matcher = processContentTypePatterns(content_type_patterns, numPatterns, patternLists->contentTypePatternList, &ctc);
    if (!content_type_matcher)
        return -1;

    return 0;
}

void http_detector_clean(void)
{
    if (via_matcher)
    {
        _dpd.searchAPI->search_instance_free(via_matcher);
        via_matcher = NULL;
    }
    if (url_matcher)
    {
        _dpd.searchAPI->search_instance_free(url_matcher);
        url_matcher = NULL;
    }
    if (client_agent_matcher)
    {
        _dpd.searchAPI->search_instance_free(client_agent_matcher);
        client_agent_matcher = NULL;
    }
    if (header_matcher)
    {
        _dpd.searchAPI->search_instance_free(header_matcher);
        header_matcher = NULL;
    }
    if (content_type_matcher)
    {
        _dpd.searchAPI->search_instance_free(content_type_matcher);
        content_type_matcher = NULL;
    }

    destroyHostUrlMatcher(&hostUrlMatcher);
    destroyHostUrlMatcher(&RTMPHostUrlMatcher);
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

void identifyUserAgent(const u_int8_t *start, int size, tAppId *serviceAppId, tAppId *clientAppId, char *version)
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

    _dpd.searchAPI->search_instance_find_all(client_agent_matcher,
               (char *)start,
               size,
               0,
               &http_pattern_match, (void *)&mp);


    if(mp)
    {
        end = start + size;
        version[0] = 0;
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
                while (i < MAX_VERSION_SIZE && buffPtr < end)
                {
                    if(*buffPtr != ' ' && *buffPtr != 0x09 && *buffPtr != ';' && *buffPtr != ')')
                       version[i++] = *buffPtr++;
                    else
                        break;
                }
                if(i == 0)
                    break;

                version[i] = 0;

                /*compatibility sheck */
                if (match->client_app == APP_ID_INTERNET_EXPLORER 
                        && strstr((char *)buffPtr, "SLCC2"))
                {
                    if ((MAX_VERSION_SIZE-i) >= (sizeof(COMPATIBLE_BROWSER_STRING) - 1))
                    {
                        strcat(version, COMPATIBLE_BROWSER_STRING);
                    } 
                }
                /* Pick firefox over some things, but pick a misc app over Firefox. */
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
                while (i < MAX_VERSION_SIZE && buffPtr < end)
                {
                    if(*buffPtr != ' ' && *buffPtr != 0x09 && *buffPtr != ';' && *buffPtr != ')')
                       version[i++] = *buffPtr++;
                    else
                        break;
                }
                if(i == 0)
                    break;

                dominant_pattern_detected = 1;
                version[i] = 0;
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
                while (i < MAX_VERSION_SIZE && buffPtr < end)
                {
                    if(*buffPtr != ' ' && *buffPtr != 0x09 && *buffPtr != ';' && *buffPtr != ')')
                       version[i++] = *buffPtr++;
                    else
                        break;
                }
                if(i == 0)
                    break;

                version[i] = 0;
                android_browser_detected = 1;
                break;

            case APP_ID_KONQUEROR:
            case APP_ID_CURL:
            case APP_ID_PICASA:
            case APP_ID_WINDOWS_MEDIA_PLAYER:
            case APP_ID_BITTORRENT:
                buffPtr = (u_int8_t*) start + tmp->index + match->pattern_size;
                if (*buffPtr != ' ' && *buffPtr != 0x09 && *buffPtr != '/') 
                    break;
                buffPtr++;
                while (i < MAX_VERSION_SIZE && buffPtr < end)
                {
                    if(*buffPtr != ' ' && *buffPtr != 0x09 && *buffPtr != ';' && *buffPtr != ')')
                       version[i++] = *buffPtr++;
                    else
                        break;
                }
                if(i == 0)
                    break;

                version[i] = 0;
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
                    while (i < MAX_VERSION_SIZE && buffPtr < end)
                    {
                        if(*buffPtr != ' ' && *buffPtr != 0x09 && *buffPtr != ';')
                           version[i++] = *buffPtr++;
                        else
                            break;
                    }
                    if(i == 0)
                        break;
                    version[i] = 0;
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
                    appleEmailDetect  = ((u_int8_t*)buffPtr && (i != 0 || (i == 0 && buffPtr == ((u_int8_t*)start))));
                }
                if (appleEmailDetect)
                {
                    dominant_pattern_detected = !(buffPtr && strstr((char *)buffPtr, SAFARI_PATTERN) != NULL);
                    version[0] = 0;
                    *serviceAppId = APP_ID_HTTP;
                    *clientAppId = match->client_app;
                }
                i = 0;
                break;

            case APP_ID_WGET:
                buffPtr = (u_int8_t*) start + tmp->index + match->pattern_size;
                while (i < MAX_VERSION_SIZE && buffPtr < end)
                {
                   version[i++] = *buffPtr++;
                }
                version[i] = 0;
                *serviceAppId = APP_ID_HTTP;
                *clientAppId = match->client_app;
                goto done;

            case APP_ID_BLACKBERRY_BROWSER:
                while( start < end && *start != '/' ) start++;
                if(start >= end)
                    break;
                start++;
                while (i < MAX_VERSION_SIZE && start < end)
                {
                    if(*start != ' ' && *start != 0x09 && *start != ';')
                    version[i++] = *start++;
                    else
                        break;
                }
                if(i == 0)
                    break;
                version[i] = 0;

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
                if (version[0])
                {
                    version[0] = 0;
                    i = 0;
                }
                buffPtr = (u_int8_t*) start + tmp->index + match->pattern_size;
                if (*buffPtr == (u_int8_t)'/')
                {
                    buffPtr++;
                    while (i < MAX_VERSION_SIZE && buffPtr < end)
                    {
                        if(*buffPtr != ' ' && *buffPtr != 0x09 && *buffPtr != ';' && *buffPtr != ')')
                            version[i++] = *buffPtr++;
                        else
                            break;
                    }
                }
                version[i] = 0;
                break;

            default:
                if (match->client_app)
                {
                    if (match->pattern_size <= longest_misc_match) break;
                    longest_misc_match = match->pattern_size;
                    i =0;
                    /* if we already collected version information after seeing 'Version', let's use that*/
                    buffPtr = (u_int8_t*) start + tmp->index + match->pattern_size;
                    /* we may have to enter the pattern with the / in it. */
                    if (*buffPtr == (u_int8_t)'/' || *buffPtr == (u_int8_t)' ')
                        buffPtr++;
                    if (buffPtr-1 > start && buffPtr < end && (*(buffPtr-1) == (u_int8_t)'/' || *(buffPtr-1) == (u_int8_t)' '))
                    {
                        while (i < MAX_VERSION_SIZE && buffPtr < end)
                        {
                            if(*buffPtr != ' ' && *buffPtr != 0x09 && *buffPtr != ';' && *buffPtr != ')')
                                version[i++] = *buffPtr++;
                            else
                                break;
                        }
                        version[i] = 0;
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
    FreeMatchStructures(mp);
}

int getAppidByViaPattern(const uint8_t *data, unsigned size, char *version)
{
    unsigned i;
    const uint8_t *data_ptr;
    const uint8_t *end = data + size;
    MatchedPatterns *mp = NULL;
    DetectorHTTPPattern *match = NULL;

    if (via_matcher)
    {
        _dpd.searchAPI->search_instance_find_all(url_matcher,
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
                    version[i++] = (char)*data_ptr;
                }
            }
            else
                i = 0;
            version[i] = 0;
            FreeMatchStructures(mp);
            return APP_ID_SQUID;

        default:
            FreeMatchStructures(mp);
            return APP_ID_NONE;
        }
    }
    return APP_ID_NONE;
}

int ScanURLForClientApp(const u_int8_t *url, int size, tAppId *clientAppId, tAppId *serviceAppId, char *clientVersion)
{
    int bt_announced = 0;
    int matched = 0;
    int skype_found = 0;
    MatchedPatterns *mp = NULL;
    MatchedPatterns *tmp;
    DetectorHTTPPattern *match;
    u_int8_t *index;
    const u_int8_t *end;

    end = url + size;

    mp = NULL;
    _dpd.searchAPI->search_instance_find_all(url_matcher,
               (char *)url,
               size, 0,
               &http_pattern_match, (void *)&mp);

    if (!mp) return matched;

    for  (tmp = mp;
          tmp;
          tmp = tmp->next)
    {
        match = (DetectorHTTPPattern *)tmp->mpattern;
        switch (match->seq)
        {
        case SKYPE_URL:
            *serviceAppId = match->service_id;
            *clientAppId = match->client_app;
            matched = 1;
            skype_found = 1;
            break;
        case SKYPE_VERSION:
            if (skype_found)
            {
                index = (u_int8_t *)url + tmp->index;
                if ((index += SKYPE_VER_SIZE) > end) break;
                for (; index <= end && *index != '&'; index++)
                {
                    *clientVersion = *index;
                    clientVersion++;
                }
                *clientVersion = 0;
            }
            break;
        /* if we get a bt announce, we need to look to see if we also caught
           one of the other bt patterns  */
        case BT_ANNOUNCE:
            bt_announced = 1;
            break;
        case BT_OTHER:
            if (bt_announced)
            {
                *serviceAppId = match->service_id;
                *clientAppId = match->client_app;
                matched = 1;
            }
            break;
        default:
            break;
        }
    }

    FreeMatchStructures(mp);

    return matched;
}

#define HTTP_HEADER_WORKINGWITH_ASPROXY "ASProxy/"

tAppId scan_header_x_working_with(const uint8_t *data, uint32_t size, char * version)
{
    uint32_t i;
    const uint8_t *end;

    version[0] = 0;

    if (size >= (sizeof(HTTP_HEADER_WORKINGWITH_ASPROXY)-1)
            && memcmp(data,HTTP_HEADER_WORKINGWITH_ASPROXY,sizeof(HTTP_HEADER_WORKINGWITH_ASPROXY)-1) == 0)
    {
        end = data+size;
        data += sizeof(HTTP_HEADER_WORKINGWITH_ASPROXY)-1;
        for (i = 0;
                data < end && i < (MAX_VERSION_SIZE-1) && *data != ')' && isprint(*data);
                data++)
        {
            version[i++] = (char)*data;
        }
        version[i] = 0;
        return APP_ID_ASPROXY;
    }
    return APP_ID_NONE;
}
tAppId getAppidByContentType(const uint8_t *data, int size)
{
    MatchedPatterns *mp = NULL;
    DetectorHTTPPattern *match;
    tAppId payloadId;

    if (content_type_matcher)
    {
        _dpd.searchAPI->search_instance_find_all(content_type_matcher,
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
    matches->last_match = target->id;
done:
    matches->last_index_end = index + target->length;
    return 0;
}

int getHTTPHeaderLocation(const uint8_t *data, unsigned size, HttpId id, int *start, int *end, HeaderMatchedPatterns *hmp)
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

    if (header_matcher)
    {
        _dpd.searchAPI->search_instance_find_all(header_matcher,
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

tAppId getAppIdFromUrl(char *host, char *url, char *payloadVersion, size_t payloadVersionLen,
                       char *referer, tAppId *clientAppId, tAppId *serviceAppId, 
                       tAppId *payloadAppId, tAppId *referredPayloadAppId, unsigned from_rtmp)
{
    char *path;
    char *referer_start;
    char *temp_host = NULL;
    const char *referer_path = NULL;
    int host_len;
    int referer_len = 0;
    int referer_path_len = 0;
    int path_len;
    tMlmpPattern patterns[3];
    tMlpPattern query;
    HostUrlDetectorPattern *data;
    char *q;
    int payload_found = 0;
    int url_len;
    static void *matcher;

#define URL_SCHEME_END_PATTERN "://"
#define URL_SCHEME_MAX_LEN     (sizeof("https://")-1)

    matcher = (from_rtmp ? RTMPHostUrlMatcher : hostUrlMatcher);

    if (!host && !url)
        return 0;

    if (url)
    {
        char *url_offset = (char *)service_strstr((uint8_t *)url, URL_SCHEME_MAX_LEN, (uint8_t *)URL_SCHEME_END_PATTERN, sizeof(URL_SCHEME_END_PATTERN)-1);
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
        path_len = url_len - host_len;
        path = url + host_len;
    }
    else
    {
        path = NULL;
        path_len = 0;
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
                query.pattern = (uint8_t *) ++q;
                query.patternSize = strlen(q);

                matchQueryElements(&query, &data->query, payloadVersion, payloadVersionLen);
            }
        }

        *clientAppId = data->client_id;
        *serviceAppId = data->service_id;
        *payloadAppId = data->payload_id;
    }

    free(temp_host);

    /* if referred_id feature id disabled, referer will be null */
    if (referer && (!payload_found || isReferredAppId(data->payload_id)))
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
                *payloadAppId = data->payload_id; 
            }
        }
    }
    return 1;
}

void getServerVendorVersion(const uint8_t *data, int len, char *version, char *vendor, RNAServiceSubtype **subtype)
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
                            subver_len = p - subver;
                            if (subver_len > 0 && *subver)
                            {
                                if ((tmp = malloc(subver_len + 1)))
                                {
                                    memcpy(tmp, subver, subver_len);
                                    tmp[subver_len] = 0;
                                    sub->version = tmp;
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
                subver_len = p - subver;
                if (subver_len > 0 && *subver)
                {
                    if ((tmp = malloc(subver_len + 1)))
                    {
                        memcpy(tmp, subver, subver_len);
                        tmp[subver_len] = 0;
                        sub->version = tmp;
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
        memcpy(version, ver, version_len);
        version[version_len] = 0;
    }
    else
        vendor_len = len;
    if (vendor_len >= MAX_VERSION_SIZE)
        vendor_len = MAX_VERSION_SIZE - 1;
    memcpy(vendor, data, vendor_len);
    vendor[vendor_len] = 0;
}

