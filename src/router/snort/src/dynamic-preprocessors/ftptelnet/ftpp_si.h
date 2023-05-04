/*
 * ftpp_si.h
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2004-2013 Sourcefire, Inc.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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

#include <stdint.h>

#include "ftpp_include.h"
#include "ftpp_ui_config.h"
#include "ftp_client.h"
#include "ftp_server.h"

#include "sf_snort_packet.h"
#include "ftpp_eo.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"
#include "session_api.h"

/*
 * These are the defines for the different types of
 * inspection modes.  We have a server mode and a client mode.
 */
#define FTPP_SI_NO_MODE     0
#define FTPP_SI_CLIENT_MODE 1
#define FTPP_SI_SERVER_MODE 2

#define FTPP_SI_PROTO_UNKNOWN   0
#define FTPP_SI_PROTO_TELNET    1
#define FTPP_SI_PROTO_FTP       2
#define FTPP_SI_PROTO_FTP_DATA  3

#define FTPP_FILE_IGNORE    -1
#define FTPP_FILE_UNKNOWN    0

/* Macros for testing the type of FTP_TELNET_SESSION */
#define FTPP_SI_IS_PROTO(Ssn, Pro)      ((Ssn) && ((Ssn)->ft_ssn.proto == (Pro)))
#define PROTO_IS_FTP(ssn)               FTPP_SI_IS_PROTO(ssn, FTPP_SI_PROTO_FTP)
#define PROTO_IS_FTP_DATA(ssn)          FTPP_SI_IS_PROTO(ssn, FTPP_SI_PROTO_FTP_DATA)
#define PROTO_IS_TELNET(ssn)            FTPP_SI_IS_PROTO(ssn, FTPP_SI_PROTO_TELNET)

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

#define DATA_CHAN_PORT_CMD_ISSUED    0x01
#define DATA_CHAN_PORT_CMD_ACCEPT    0x02
#define DATA_CHAN_PASV_CMD_ISSUED    0x04
#define DATA_CHAN_PASV_CMD_ACCEPT    0x08
#define DATA_CHAN_XFER_CMD_ISSUED    0x10
#define DATA_CHAN_XFER_STARTED       0x20
#define DATA_CHAN_CLIENT_HELLO_SEEN  0x40
#define DATA_CHAN_REST_CMD_ISSUED    0x80

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
    tSfPolicyId policy_id;

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
    tSfPolicyUserContextId global_conf;

    /* The data channel info */
    int data_chan_state;
    uint32_t data_chan_index;
    uint32_t data_xfer_index;
    sfaddr_t      clientIP;
    uint16_t clientPort;
    sfaddr_t      serverIP;
    uint16_t serverPort;
    uint32_t ftp_cmd_pipe_index;
    uint32_t rest_cmd_offset;
    uint16_t control_clientPort;
    uint16_t control_serverPort;

    /* A file is being transfered on ftp-data channel */
    char *filename;
    int file_xfer_info; /* -1: ignore, 0: unknown, >0: filename length */
    bool data_xfer_dir;

    /* Command/data channel encryption */
    bool encr_state_chello;
    unsigned char flags;
    int encr_state;
    uint32_t flow_id;

    /* Alertable event list */
    FTP_EVENTS event_list;
    void *datassn;
    sfaddr_t      control_clientIP;
    sfaddr_t      control_serverIP;

} FTP_SESSION;

#define FTP_FLG_MALWARE_ENABLED (1<<1)

#ifdef TARGET_BASED

/* FTP-Data Transfer Modes */
enum {
    FTPP_XFER_PASSIVE = 0,
    FTPP_XFER_ACTIVE  = 1
};

typedef struct s_FTP_DATA_SESSION
{
    FTP_TELNET_SESSION ft_ssn;
    StreamSessionKey * ftp_key;
    void* ftpssn;
    char *filename;
    int data_chan;
    int file_xfer_info;
    FilePosition position;
    bool direction;
    unsigned char mode;
    unsigned char flags;
    uint32_t flow_id;
    uint32_t path_hash;
} FTP_DATA_SESSION;

#define FTPDATA_FLG_REASSEMBLY_SET  (1<<0)
#define FTPDATA_FLG_FILENAME_SET    (1<<1)
#define FTPDATA_FLG_STOP            (1<<2)
#define FTPDATA_FLG_REST            (1<<3)
#define FTPDATA_FLG_FLUSH           (1<<4)

#endif

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
    sfaddr_t sip;
    sfaddr_t dip;
    unsigned short sport;
    unsigned short dport;
    unsigned char pdir;
    unsigned char pproto;

} FTPP_SI_INPUT;

int ftpp_si_determine_proto(SFSnortPacket *p, FTPTELNET_GLOBAL_CONF *GlobalConf,
        FTP_TELNET_SESSION **, FTPP_SI_INPUT *SiInput, int *piInspectMode);
int FTPGetPacketDir(SFSnortPacket *);

#ifdef TARGET_BASED
/* FTP-Data file processing */
FTP_DATA_SESSION * FTPDataSessionNew(SFSnortPacket *p);
void FTPDataSessionFree(void *p_ssn);
bool FTPDataDirection(SFSnortPacket *p, FTP_DATA_SESSION *ftpdata);
#endif

#endif /* ! __FTPP_SI_H__ */

