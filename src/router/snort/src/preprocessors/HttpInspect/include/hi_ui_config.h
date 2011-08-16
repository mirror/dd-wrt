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
**  @file       hi_ui_config.h
**  
**  @author     Daniel Roelker <droelker@sourcefire.com>
**
**  @brief      This file contains the internal configuration structures
**              for HttpInspect.
**
**  This file holds the configuration constructs for the HttpInspect global
**  configuration and the server configurations.  It also contains the function
**  prototypes for accessing server configurations.
*/

#ifndef __HI_UI_CONFIG_H__
#define __HI_UI_CONFIG_H__

#include "hi_include.h"
#include "sfrt.h"
#include "ipv6_port.h"
#include "sf_ip.h"
#include "sfPolicy.h"
#include "hi_util_kmap.h"

/*
**  Defines
*/
#define HI_UI_CONFIG_STATELESS 0
#define HI_UI_CONFIG_STATEFUL  1
#define HI_UI_CONFIG_MAX_PIPE  20

#define HI_UI_CONFIG_MAX_HDR_DEFAULT 0
#define HI_UI_CONFIG_MAX_HEADERS_DEFAULT 0

/*
**  Special characters treated as whitespace before or after URI
*/

#define HI_UI_CONFIG_WS_BEFORE_URI 0x01
#define HI_UI_CONFIG_WS_AFTER_URI  0x02

/**Maximum number of entries in server_lookup table.
*/
#define HI_UI_CONFIG_MAX_SERVERS 1025

/**
**  Defines a search type for the server configurations in the
**  global configuration.  We want this generic so we can change
**  it easily if we change the search type.
*/
typedef table_t SERVER_LOOKUP;

/**
**  This structure simply holds a value for on/off and whether
**  alert is on/off.  Should be used for many configure options.
*/
typedef struct s_HTTPINSPECT_CONF_OPT
{

    int on;     /**< if true, configuration option is on */
    int alert;  /**< if true, alert if option is found */

}  HTTPINSPECT_CONF_OPT;

/* The following are used to delineate server profiles for user output
 * and debugging information. */
typedef enum e_PROFILES 
{
    HI_ALL,
    HI_APACHE,
    HI_IIS,
    HI_IIS4,
    HI_IIS5
} PROFILES;

typedef KMAP CMD_LOOKUP;

typedef struct s_HTTP_CMD_CONF
{
    char cmd_name[1];  // variable length array

}  HTTP_CMD_CONF;

/**
**  This is the configuration construct that holds the specific
**  options for a server.  Each unique server has it's own structure
**  and there is a global structure for servers that don't have
**  a unique configuration.
*/
typedef struct s_HTTPINSPECT_CONF
{
    int  port_count;
    char ports[MAXPORTS_STORAGE];
    int  server_flow_depth;
    int  client_flow_depth;
    int  post_depth;

    /*
    **  Unicode mapping for IIS servers
    */
    int  *iis_unicode_map;
    char *iis_unicode_map_filename;
    int  iis_unicode_codepage;

    int  long_dir;
    
    /*
    **  Chunk encoding anomaly detection
    */
    unsigned int chunk_length;

    char uri_only;
    char no_alerts;
    char enable_cookie;
    char inspect_response;
    char enable_xff;

#ifdef ZLIB
    char extract_gzip;
    char unlimited_decompress;
#endif
   
   /* Support Extended ascii codes in the URI */ 
    char extended_ascii_uri;
    /*
    **  pipeline requests
    */
    char no_pipeline;

    /*
    **  Enable non-strict (apache) URI handling.  This allows us to catch the
    **  non-standard URI parsing that apache does.
    */
    char non_strict;

    /*
    **  Allow proxy use for this server.
    */
    char allow_proxy;

    /*
    **  Handle tab char (0x09) as a URI delimiter.  Apache honors this, IIS does not.
    */
    char tab_uri_delimiter;

    /*
    **  Normalize HTTP Headers if they exist.  
    XXX Not sure what Apache & IIS do with respect to HTTP header 'uri' normalization.
    */
    char normalize_headers;

    /*
    **  Normalize HTTP Headers if they exist.  
    XXX Not sure what Apache & IIS do with respect to HTTP header 'uri' normalization.
    */
    char normalize_cookies;

    /*
    **  Normalize multi-byte UTF charsets in HTTP server responses.
    */
    char normalize_utf;

    /*
    **  Characters to be treated as whitespace bracketing a URI.
    */
    char whitespace[256];

    /*
    **  These are the URI encoding configurations
    */
    HTTPINSPECT_CONF_OPT ascii;
    HTTPINSPECT_CONF_OPT double_decoding;
    HTTPINSPECT_CONF_OPT u_encoding;
    HTTPINSPECT_CONF_OPT bare_byte;
    HTTPINSPECT_CONF_OPT base36;
    HTTPINSPECT_CONF_OPT utf_8;
    HTTPINSPECT_CONF_OPT iis_unicode;
    char                 non_rfc_chars[256];

    /*
    **  These are the URI normalization configurations
    */
    HTTPINSPECT_CONF_OPT multiple_slash;
    HTTPINSPECT_CONF_OPT iis_backslash;
    HTTPINSPECT_CONF_OPT directory;
    HTTPINSPECT_CONF_OPT webroot;
    HTTPINSPECT_CONF_OPT apache_whitespace;
    HTTPINSPECT_CONF_OPT iis_delimiter;
    int max_hdr_len;
    int max_headers;

    PROFILES profile;
    CMD_LOOKUP    *cmd_lookup;
    
    /**Used to track references to this allocated data structure. Each additional
     * reference should increment referenceCount. Each attempted free should 
     * decrement it. When free is attempted and reference count is 0, then 
     * this HTTPINSPECT_CONF should be actually freed. 
     */ 
    int referenceCount;

}  HTTPINSPECT_CONF;

/**
**  This is the configuration for the global HttpInspect
**  configuration.  It contains the global aspects of the
**  configuration, a standard global default configuration,
**  and server configurations.
*/
typedef struct s_HTTPINSPECT_GLOBAL_CONF
{
#ifdef ZLIB
    int              disabled;
#endif
    int              max_pipeline_requests;
    int              inspection_type;
    int              anomalous_servers;
    int              proxy_alert;

    /*
    **  These variables are for tracking the IIS
    **  Unicode Map configuration.
    */
    int              *iis_unicode_map;
    char             *iis_unicode_map_filename;
    int              iis_unicode_codepage;

    HTTPINSPECT_CONF *global_server;
    SERVER_LOOKUP    *server_lookup;


#ifdef ZLIB
    int max_gzip_sessions;
    int max_gzip_mem;
    int compr_depth;
    int decompr_depth;
#endif

}  HTTPINSPECT_GLOBAL_CONF;    

#define INVALID_HEX_VAL -1
#define HEX_VAL          1
#define BASE36_VAL      -2

/*
**  Functions
*/
int hi_ui_config_init_global_conf(HTTPINSPECT_GLOBAL_CONF *GlobalConf);
int hi_ui_config_default(HTTPINSPECT_CONF *GlobalConf);
int hi_ui_config_reset_global(HTTPINSPECT_GLOBAL_CONF *GlobalConf);
int hi_ui_config_reset_server(HTTPINSPECT_CONF *ServerConf);

int hi_ui_config_add_server(HTTPINSPECT_GLOBAL_CONF *GlobalConf,
                            sfip_t *ServerIP,
                            HTTPINSPECT_CONF *ServerConf);

int hi_ui_config_set_profile_apache(HTTPINSPECT_CONF *GlobalConf);
int hi_ui_config_set_profile_iis(HTTPINSPECT_CONF *GlobalConf, int *);
int hi_ui_config_set_profile_iis_4or5(HTTPINSPECT_CONF *GlobalConf, int *);
int hi_ui_config_set_profile_all(HTTPINSPECT_CONF *GlobalConf, int *);
void HttpInspectCleanupHttpMethodsConf(void *);

extern int hex_lookup[256];
extern int valid_lookup[256];

#endif
