/*
 * ftpp_ui_config.c
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
 * This file contains library calls to configure FTPTelnet.
 *
 * This file deals with configuring FTPTelnet processing.  It contains
 * routines to set a default configuration, add client configurations, etc.
 *
 * NOTES:
 * - 16.09.04:  Initial Development.  SAS
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ftpp_return_codes.h"
#include "ftpp_ui_client_lookup.h"
#include "ftpp_ui_server_lookup.h"
#include "ftp_cmd_lookup.h"
#include "ftp_bounce_lookup.h"
#include "ftpp_ui_config.h"
#include "memory_stats.h"

/*
 * Function: ftpp_ui_config_init_global_conf(FTPTELNET_GLOBAL_CONF *GlobalConf)
 *
 * Purpose: Initialize the FTPTelnet global configuration.
 *          The main point of this function is to initialize the client
 *          lookup type.  We also do things like memset, etc.
 *
 * Arguments: GlobalConf    => pointer to the global configuration
 *
 * Returns: int => return code indicating error or success
 *
 */
int ftpp_ui_config_init_global_conf(FTPTELNET_GLOBAL_CONF *GlobalConf)
{
    int iRet;

    iRet = ftpp_ui_client_lookup_init(&GlobalConf->client_lookup);
    if (iRet)
    {
        return iRet;
    }

    iRet = ftpp_ui_server_lookup_init(&GlobalConf->server_lookup);
    if (iRet)
    {
        return iRet;
    }

    return FTPP_SUCCESS;
}

/*
 * Function: ftpp_ui_config_default(FTPTELNET_GLOBAL_CONF *GlobalConf)
 *
 * Purpose: This function sets the global and the global_client default
 *          configuration.  In order to change the default configuration
 *          of FTPTelnet, you must change this function.
 *
 * Arguments: GlobalConf    => pointer to the global configuration structure
 *
 * Returns: int => return code indicating error or success
 *
 */
int ftpp_ui_config_default(FTPTELNET_GLOBAL_CONF *GlobalConf)
{
    if(GlobalConf == NULL)
    {
        return FTPP_INVALID_ARG;
    }

    /*
     * Set Global Client Configurations
     */
    ftpp_ui_config_reset_ftp_client(GlobalConf->default_ftp_client, 0);
    ftpp_ui_config_reset_ftp_server(GlobalConf->default_ftp_server, 0);
    ftpp_ui_config_reset_telnet_proto(GlobalConf->telnet_config);

    return FTPP_SUCCESS;
}

/*
 * Function: ftpp_ui_config_reset_global(FTPTELNET_GLOBAL_CONF *GlobalConf)
 *
 * Purpose: This function resets the global parameters.
 *          THIS IS NOT THE GLOBAL FTP CLIENT CONFIGURATION.
 *
 * Arguments: GlobalConf    => pointer to the global configuration structure
 *
 * Returns: int => return code indicating error or success
 *
 */
int ftpp_ui_config_reset_global(FTPTELNET_GLOBAL_CONF *GlobalConf)
{
    int iRet;

    /* Clean these up before mem setting */
    ftp_bounce_lookup_cleanup(&GlobalConf->default_ftp_client->bounce_lookup);
    ftp_cmd_lookup_cleanup(&(GlobalConf->default_ftp_server->cmd_lookup));

    ftpp_ui_client_lookup_cleanup(&GlobalConf->client_lookup);
    ftpp_ui_server_lookup_cleanup(&GlobalConf->server_lookup);

    memset(GlobalConf, 0x00, sizeof(FTPTELNET_GLOBAL_CONF));

    iRet = ftpp_ui_client_lookup_init(&GlobalConf->client_lookup);
    if (iRet)
    {
        return iRet;
    }

    iRet = ftpp_ui_server_lookup_init(&GlobalConf->server_lookup);
    if (iRet)
    {
        return iRet;
    }

    return FTPP_SUCCESS;
}

/*
 * Function: ftpp_ui_config_reset_telnet_proto(TELNET_PROTO_CONF *TelnetConf)
 *
 * Purpose: This function resets a telnet construct.
 *
 * Arguments: TelnetConf    => pointer to the TELNET_PROTO_CONF structure
 *
 * Returns: int => return code indicating error or success
 *
 */
int ftpp_ui_config_reset_telnet_proto(TELNET_PROTO_CONF *TelnetConf)
{
    memset(TelnetConf, 0x00, sizeof(TELNET_PROTO_CONF));

    TelnetConf->ayt_threshold = FTPP_UI_CONFIG_TELNET_DEF_AYT_THRESHOLD;

    TelnetConf->proto_ports.port_count = 1;
    TelnetConf->proto_ports.ports[23] = 1;

    return FTPP_SUCCESS;
}

/*
 * Function: ftpp_ui_config_reset_ftp_cmd_date_format(FTP_DATE_FMT *DateFmt)
 *
 * Purpose: This function resets an FTP date parameter construct.
 *
 * Arguments: ThisFmt   => pointer to the FTP_DATE_FMT structure
 *
 * Returns: void
 *
 */
void ftpp_ui_config_reset_ftp_cmd_date_format(FTP_DATE_FMT *DateFmt)
{
    if (DateFmt->optional)
    {
        ftpp_ui_config_reset_ftp_cmd_date_format(DateFmt->optional);
    }

    if (DateFmt->next)
    {
        ftpp_ui_config_reset_ftp_cmd_date_format(DateFmt->next);
    }

    if (DateFmt->format_string)
    {
        _dpd.snortFree(DateFmt->format_string,
                       (strlen(DateFmt->format_string) + 1),
                       PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
    }
    _dpd.snortFree(DateFmt, sizeof(FTP_DATE_FMT),
                   PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
}

/*
 * Function: ftpp_ui_config_reset_ftp_cmd_format(FTP_PARAM_FMT *ThisFmt)
 *
 * Purpose: This function resets an FTP parameter construct.
 *
 * Arguments: ThisFmt   => pointer to the FTP_PARAM_FMT structure
 *
 * Returns: void
 *
 */
void ftpp_ui_config_reset_ftp_cmd_format(FTP_PARAM_FMT *ThisFmt)
{
    if (ThisFmt->optional_fmt)
    {
        ftpp_ui_config_reset_ftp_cmd_format(ThisFmt->optional_fmt);
    }
    if (ThisFmt->numChoices)
    {
        int i;
        for (i=0;i<ThisFmt->numChoices;i++)
        {
            ftpp_ui_config_reset_ftp_cmd_format(ThisFmt->choices[i]);
        }
        _dpd.snortFree(ThisFmt->choices,
                       (ThisFmt->numChoices * sizeof(FTP_PARAM_FMT *)),
                       PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
    }

    if (ThisFmt->next_param_fmt)
    {
        /* Don't free this one twice if its after an optional */
        FTP_PARAM_FMT *next = ThisFmt->next_param_fmt;
        ThisFmt->next_param_fmt->prev_param_fmt->next_param_fmt = NULL;
        ThisFmt->next_param_fmt = NULL;
        ftpp_ui_config_reset_ftp_cmd_format(next);
    }

    if (ThisFmt->type == e_date)
    {
        ftpp_ui_config_reset_ftp_cmd_date_format(ThisFmt->format.date_fmt);
    }
    if (ThisFmt->type == e_literal)
    {
        _dpd.snortFree(ThisFmt->format.literal,
                       (strlen(ThisFmt->format.literal) + 1),
                       PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
    }

    memset(ThisFmt, 0, sizeof(FTP_PARAM_FMT));
    _dpd.snortFree(ThisFmt, sizeof(FTP_PARAM_FMT),
                   PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
}

/*
 * Function: ftpp_ui_config_reset_ftp_cmd(FTP_CMD_CONF *FTPCmd)
 *
 * Purpose: This function resets a FTP command construct.
 *
 * Arguments: FTPCmd    => pointer to the FTP_CMD_CONF structure
 *
 * Returns: int => return code indicating error or success
 *
 */
int ftpp_ui_config_reset_ftp_cmd(FTP_CMD_CONF *FTPCmd)
{
    FTP_PARAM_FMT *NextCmdFormat = FTPCmd->param_format;

    if (NextCmdFormat)
    {
        ftpp_ui_config_reset_ftp_cmd_format(NextCmdFormat);
    }

    return FTPP_SUCCESS;
}

/*
 * Function: ftpp_ui_config_reset_ftp_server(FTP_SERVER_PROTO_CONF *ServerConf,
 *                                  char first)
 *
 * Purpose: This function resets a ftp server construct.
 *
 * Arguments: ServerConf    => pointer to the FTP_SERVER_PROTO_CONF structure
 *            first         => indicator whether this is a new conf
 *
 * Returns: int => return code indicating error or success
 *
 */
int ftpp_ui_config_reset_ftp_server(FTP_SERVER_PROTO_CONF *ServerConf,
                                    char first)
{
    int iRet = FTPP_SUCCESS;

    if (first == 0)
    {
        ftp_cmd_lookup_cleanup(&ServerConf->cmd_lookup);
    }
    if (ServerConf->serverAddr)
    {
        free(ServerConf->serverAddr);
    }

    memset(ServerConf, 0x00, sizeof(FTP_SERVER_PROTO_CONF));

    ServerConf->proto_ports.port_count = 1;
    ServerConf->proto_ports.ports[21] = 1;

    ftp_cmd_lookup_init(&ServerConf->cmd_lookup);

    ServerConf->def_max_param_len = FTPP_UI_CONFIG_FTP_DEF_CMD_PARAM_MAX;
    ServerConf->max_cmd_len = MAX_CMD;

    return iRet;
}

/*
 * Function: ftpp_ui_config_add_ftp_server(
 *                          FTPTELNET_GLOBAL_CONF *GlobalConf,
 *                          unsigned long ServerIP,
 *                          FTP_SERVER_PROTO_CONF *ServerConf)
 *
 * Purpose: Add a server config to the FTPTelnet configuration.
 *          This function takes an IP address of a server and an FTP
 *          Server configuration, and assigns the configuration to
 *          the IP address in a lookup table.
 *
 * Arguments: GlobalConf    => pointer to the global configuration
 *            ServerIp      => the IP address of the server
 *            ServerConf    => pointer to the server configuration
 *
 * Returns: int => return code indicating error or success
 *
 */
int ftpp_ui_config_add_ftp_server(FTPTELNET_GLOBAL_CONF *GlobalConf,
                            sfcidr_t * ServerIP, FTP_SERVER_PROTO_CONF *ServerConf)
{
    int iRet;

    iRet = ftpp_ui_server_lookup_add(GlobalConf->server_lookup, ServerIP, ServerConf);
    if (iRet)
    {
        /*
         * Already added key will return a generic non-fatal
         * error.
         */
        return iRet;
    }

    return FTPP_SUCCESS;
}

/*
 * Function: ftpp_ui_config_reset_ftp_client(FTP_CLIENT_PROTO_CONF *ClientConf,
 *                                  char first)
 *
 * Purpose: This function resets a ftp client construct.
 *
 * Arguments: ClientConf    => pointer to the FTP_CLIENT_PROTO_CONF structure
 *            first         => indicator whether this is a new conf
 *
 *
 * Returns: int => return code indicating error or success
 *
 */
int ftpp_ui_config_reset_ftp_client(FTP_CLIENT_PROTO_CONF *ClientConf,
                                    char first)
{
    int iRet = FTPP_SUCCESS;
    //FTP_BOUNCE_TO *NextBounceTo = NULL;

    if (first == 0)
    {
        ftp_bounce_lookup_cleanup(&ClientConf->bounce_lookup);
    }
    if (ClientConf->clientAddr)
    {
        free(ClientConf->clientAddr);
    }

    memset(ClientConf, 0x00, sizeof(FTP_CLIENT_PROTO_CONF));

    ftp_bounce_lookup_init(&ClientConf->bounce_lookup);

    ClientConf->max_resp_len = (unsigned int)FTPP_UI_CONFIG_FTP_DEF_RESP_MSG_MAX;

    return iRet;
}

/*
 * Function: ftpp_ui_config_add_ftp_client(
 *                          FTPTELNET_GLOBAL_CONF *GlobalConf,
 *                          unsigned long ClientIP,
 *                          FTP_SERVER_PROTO_CONF *ClientConf)
 *
 * Purpose: Add a client config to the FTPTelnet configuration.
 *          This function takes an IP address of a client and an FTP
 *          Client configuration, and assigns the configuration to
 *          the IP address in a lookup table.
 *
 * Arguments: GlobalConf    => pointer to the global configuration
 *            ClientIP      => the IP address of the client
 *            ClientConf    => pointer to the client configuration
 *
 * Returns: int => return code indicating error or success
 *
 */
int ftpp_ui_config_add_ftp_client(FTPTELNET_GLOBAL_CONF *GlobalConf,
                            sfcidr_t * ClientIP, FTP_CLIENT_PROTO_CONF *ClientConf)
{
    int iRet;

    iRet = ftpp_ui_client_lookup_add(GlobalConf->client_lookup, ClientIP, ClientConf);
    if (iRet)
    {
        /*
         * Already added key will return a generic non-fatal
         * error.
         */
        return iRet;
    }

    return FTPP_SUCCESS;
}
