/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2003-2013 Sourcefire, Inc.
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
 ****************************************************************************/

/**
**  @file       hi_si.h
**
**  @author     Daniel J. Roelker <droelker@sourcefire.com>
**
**  @brief      This file contains structures and functions for the
**              Session Inspection Module.
**
**  The Session Inspection Module has several data structures that are
**  very important to the functionality of the module.  The two major
**  structures are the HI_SESSION and the HI_SI_INPUT.
**
**  NOTES:
**  - 2.25.03:  Initial Development.  DJR
*/
#ifndef __HI_SI_H__
#define __HI_SI_H__

#include "hi_include.h"
#include "hi_ui_config.h"
#include "hi_client.h"
#include "hi_server.h"
#include "hi_ad.h"

#include "ipv6_port.h"
#include "decode.h"

/*
**  These are the defines for the different types of
**  inspection modes.  We have a server mode, client mode and a "no" mode which
**  looks for anomalous HTTP server detection and tunneling.
*/
#define HI_SI_NO_MODE     0
#define HI_SI_CLIENT_MODE 1
#define HI_SI_SERVER_MODE 2

/**
**  The HI_SESSION structure contains the complete HTTP session, both the
**  client and the server constructs.  This structure is the structure that
**  is saved per session in the Stream Interface Module.  This structure
**  gets sent through the detection engine process (Normalization,
**  Detection).
*/
typedef struct s_HI_SESSION
{
    /*
    **  The client construct contains all the info associated with a
    **  client request.
    */
    HI_CLIENT client;

    /*
    **  The server construct contains all the info associated with a
    **  server response.
    */
    HI_SERVER server;

    /*
    **  The anomalous server construct that let's us do things when we've
    **  found undefined HTTP traffic.
    */
    HI_ANOM_SERVER anom_server;

    /*
    **  The server configuration for this session
    */
    HTTPINSPECT_CONF *server_conf;

    /*
    **  If this HTTP request came from a proxy, we
    **  have to see if it was configured.
    */
    HTTPINSPECT_CONF *client_conf;

    /*
    **  The global configuration for this session
    */
    HTTPINSPECT_GLOBAL_CONF *global_conf;

    uint32_t norm_flags;

} HI_SESSION;

#define HI_BODY 1

/**
**  The HI_SI_INPUT structure holds the information that the Session
**  Inspection Module needs to determine the type of inspection mode
**  (client, server, neither) and to retrieve the appropriate server
**  configuration.
**
**  The input is the source and destination IP addresses, and the
**  source and destination ports (since this should always be a
**  TCP packet).
*/
typedef struct s_HI_SI_INPUT
{
    sfaddr_t sip;
    sfaddr_t dip;
    unsigned short sport;
    unsigned short dport;
    unsigned char pdir;

} HI_SI_INPUT;

int hi_si_session_inspection(HTTPINSPECT_GLOBAL_CONF *GlobalConf,
        HI_SESSION **Session, HI_SI_INPUT *SiInput, int *piInspectMode,
        Packet *p);
int GetHttpConf(HTTPINSPECT_GLOBAL_CONF *GlobalConf,HTTPINSPECT_CONF **ServerConf,
        HTTPINSPECT_CONF **ClientConf,HI_SI_INPUT *SiInput, int *piInspectMode, void *ssnptr);

#endif

