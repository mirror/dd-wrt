/*
 * snort_ftptelnet.h
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
 * This file defines the publicly available functions for the FTPTelnet
 * functionality for Snort.
 *
 * NOTES:
 * - 16.09.04:  Initial Development.  SAS
 *
 */
#ifndef __SNORT_FTPTELNET_H__
#define __SNORT_FTPTELNET_H__

#include "ftpp_ui_config.h"
#include "sf_snort_packet.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"

/*
 * The definition of the configuration separators in the snort.conf
 * configure line.
 */
#define CONF_SEPARATORS " \t\n\r"

/*
 * These are the definitions of the parser section delimiting
 * keywords to configure FtpTelnet.  When one of these keywords
 * are seen, we begin a new section.
 */
#define GLOBAL        "global"
#define TELNET        "telnet"
#define FTP           "ftp"
//#define GLOBAL_CLIENT "global_client"
#define CLIENT        "client"
#define SERVER        "server"

#ifdef TARGET_BASED
extern int16_t ftp_app_id;
extern int16_t ftp_data_app_id;
extern int16_t telnet_app_id;
#endif

void FTPTelnetFreeConfigs(tSfPolicyUserContextId GlobalConf);
void FTPTelnetFreeConfig(FTPTELNET_GLOBAL_CONF *GlobalConf);
int SnortFTPTelnet(SFSnortPacket *p);
#ifdef TARGET_BASED
void SnortFTPData_EOF(SFSnortPacket *p);
void SnortFTPData_Flush(SFSnortPacket *p);
int SnortFTPData(SFSnortPacket *p);
#endif
int FTPConfigCheck(struct _SnortConfig *);
int FtpTelnetInitGlobalConfig(FTPTELNET_GLOBAL_CONF *, char *, int);
char *NextToken(char *delimiters);

int FTPPBounceInit(struct _SnortConfig *sc, char *name, char *parameters, void **dataPtr);
int FTPPBounceEval(void *p, const uint8_t **cursor, void *dataPtr);

void FTPTelnetChecks(void *pkt, void *context);
#ifdef TARGET_BASED
void FTPDataTelnetChecks(void *pkt, void *context);
#endif

void FTPTelnetCleanupFTPServerConf(void *serverConf);
void FTPTelnetCleanupFTPCMDConf(void *ftpCmd);
void FTPTelnetCleanupFTPClientConf(void *clientConf);
void FTPTelnetCleanupFTPBounceTo(void *ftpBounce);
int FTPTelnetCheckFTPServerConfigs(struct _SnortConfig *, FTPTELNET_GLOBAL_CONF *);

int ProcessFTPGlobalConf(FTPTELNET_GLOBAL_CONF *, char *, int);
int ProcessTelnetConf(FTPTELNET_GLOBAL_CONF *, char *, int);
int ProcessFTPClientConf(struct _SnortConfig *sc, FTPTELNET_GLOBAL_CONF *, char *, int);
int ProcessFTPServerConf(struct _SnortConfig *sc, FTPTELNET_GLOBAL_CONF *, char *, int);
int PrintFTPGlobalConf(FTPTELNET_GLOBAL_CONF *);
int FTPTelnetCheckConfigs(struct _SnortConfig *, void* , tSfPolicyId );
void enableFtpTelnetPortStreamServices( struct _SnortConfig *sc, PROTO_CONF *pc, char *network, int direction );
#endif
