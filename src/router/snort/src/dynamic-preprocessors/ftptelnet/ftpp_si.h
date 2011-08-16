/*
 * ftpp_si.h
 *
 * Copyright (C) 2004-2011 Sourcefire, Inc.
 * Steven A. Sturges <ssturges@sourcefire.com>
 * Daniel J. Roelker <droelker@sourcefire.com>
 * Marc A. Norton <mnorton@sourcefire.com>
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
 * Description:
 *
 * This file contains structures and functions for the
 * Session Inspection Module.
 *
 * The Session Inspection Module has several data structures that are
 * very important to the functionality of the module.  The two major
 * structures are the FTPP_SESSION and the FTPP_SI_INPUT.
 *
 * NOTES:
 * - 20.09.04:  Initial Development.  SAS
 *
 */
#ifndef __FTPP_SI_H__
#define __FTPP_SI_H__

#include "ftpp_include.h"
#include "ftpp_ui_config.h"
#include "ftp_client.h"
#include "ftp_server.h"
//#include "decode.h"
#include "sf_snort_packet.h"
#include "ftpp_eo.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"

/*
 * These are the defines for the different types of
 * inspection modes.  We have a server mode and a client mode.
 */
#define FTPP_SI_NO_MODE     0
#define FTPP_SI_CLIENT_MODE 1
#define FTPP_SI_SERVER_MODE 2

#define FTPP_SI_PROTO_UNKNOWN 0
#define FTPP_SI_PROTO_TELNET  1
#define FTPP_SI_PROTO_FTP     2

typedef struct s_FTP_TELNET_SESSION
{
    int proto;

} FTP_TELNET_SESSION;

/*
 * The TELNET_SESSION structure contains the complete TELNET session.  
 * This structure is the structure that is saved per session in the
 * Stream Interface Module.  This structure gets sent through the
 * detection engine process (Normalization, Detection).
 */
typedef struct s_TELNET_SESSION
{
    FTP_TELNET_SESSION ft_ssn;

    /* The global configuration for this session */
    tSfPolicyId policy_id;
    tSfPolicyUserContextId global_conf;

    /* The client configuration for this session if its FTP */
    TELNET_PROTO_CONF *telnet_conf;

    /* Number of consecutive are-you-there commands seen. */
    int consec_ayt;

    int encr_state;

    TELNET_EVENTS event_list;

} TELNET_SESSION;

/*
 * These are the state values for determining the FTP data channel.
 */
#define NO_STATE                  0x00
#define LOST_STATE                0xFFFFFFFF

#define DATA_CHAN_PORT_CMD_ISSUED 0x01
#define DATA_CHAN_PORT_CMD_ACCEPT 0x02
#define DATA_CHAN_PASV_CMD_ISSUED 0x04
#define DATA_CHAN_PASV_CMD_ACCEPT 0x08
#define DATA_CHAN_XFER_CMD_ISSUED 0x10
#define DATA_CHAN_XFER_STARTED    0x20

#define AUTH_TLS_CMD_ISSUED       0x01
#define AUTH_SSL_CMD_ISSUED       0x02
#define AUTH_UNKNOWN_CMD_ISSUED   0x04
#define AUTH_TLS_ENCRYPTED        0x08
#define AUTH_SSL_ENCRYPTED        0x10
#define AUTH_UNKNOWN_ENCRYPTED    0x20

/*
 * The FTP_SESSION structure contains the complete FTP session, both the
 * client and the server constructs.  This structure is the structure that 
 * is saved per session in the Stream Interface Module.  This structure 
 * gets sent through the detection engine process (Normalization, 
 * Detection).
 */
typedef struct s_FTP_SESSION
{
    FTP_TELNET_SESSION ft_ssn;

    /* The client construct contains all the info associated with a 
     * client request. */
    FTP_CLIENT client;

    /* The server construct contains all the info associated with a 
     * server response. */
    FTP_SERVER server;

    /* The client configuration for this session if its FTP */
    FTP_CLIENT_PROTO_CONF *client_conf;

    /* The server configuration for this session if its FTP */
    FTP_SERVER_PROTO_CONF *server_conf;

    /* The global configuration for this session */
    tSfPolicyId policy_id;
    tSfPolicyUserContextId global_conf;

    /* The data channel info */
    int data_chan_state;
    int data_chan_index;
    int data_xfer_index;
    snort_ip      clientIP;
    uint16_t clientPort;
    snort_ip      serverIP;
    uint16_t serverPort;

    /* Command/data channel encryption */
    int encr_state;

    /* Alertable event list */
    FTP_EVENTS event_list;

} FTP_SESSION;

/*
 * The FTPP_SI_INPUT structure holds the information that the Session
 * Inspection Module needs to determine the type of inspection mode
 * (client, server, neither) and to retrieve the appropriate server
 * configuration.
 *
 * The input is the source and destination IP addresses, and the 
 * source and destination ports (since this should always be a
 * TCP packet).
 */
typedef struct s_FTPP_SI_INPUT
{
    snort_ip sip;
    snort_ip dip;
    unsigned short sport;
    unsigned short dport;
    unsigned char pdir;
    unsigned char pproto;

} FTPP_SI_INPUT;

int ftpp_si_determine_proto(SFSnortPacket *p, FTPTELNET_GLOBAL_CONF *GlobalConf,
        FTP_TELNET_SESSION **, FTPP_SI_INPUT *SiInput, int *piInspectMode);
int FTPGetPacketDir(SFSnortPacket *);

#endif

