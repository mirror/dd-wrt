/*
 * ftpp_eo_events.h
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
 * Defines the events for the FTP Telnet Preprocessor.
 *
 * NOTES:
 * - 20.09.04:  Initial Development.  SAS
 *
 */
#ifndef __FTP_EO_EVENTS_H__
#define __FTP_EO_EVENTS_H__

#include "ftpp_include.h"

/*
 * FTP Events
 */
#define FTP_EO_TELNET_CMD                       0
#define FTP_EO_INVALID_CMD                      1
#define FTP_EO_PARAMETER_LENGTH_OVERFLOW        2
#define FTP_EO_MALFORMED_PARAMETER              3
#define FTP_EO_PARAMETER_STR_FORMAT             4
#define FTP_EO_RESPONSE_LENGTH_OVERFLOW         5
#define FTP_EO_ENCRYPTED                        6
#define FTP_EO_BOUNCE                           7
#define FTP_EO_EVASIVE_TELNET_CMD               8

#define FTP_EO_TELNET_CMD_SID                   1
#define FTP_EO_INVALID_CMD_SID                  2
#define FTP_EO_PARAMETER_LENGTH_OVERFLOW_SID    3
#define FTP_EO_MALFORMED_PARAMETER_SID          4
#define FTP_EO_PARAMETER_STR_FORMAT_SID         5
#define FTP_EO_RESPONSE_LENGTH_OVERFLOW_SID     6
#define FTP_EO_ENCRYPTED_SID                    7
#define FTP_EO_BOUNCE_SID                       8
#define FTP_EO_EVASIVE_TELNET_CMD_SID           9

/*
 * IMPORTANT:
 * Every time you add an FTP event, this number must be
 * incremented.
 */
#define FTP_EO_EVENT_NUM      9

/*
 * These defines are the alert names for each event
 */
#define FTP_EO_TELNET_CMD_STR                       \
    "(ftp_telnet) TELNET CMD on FTP Command Channel"
#define FTP_EO_INVALID_CMD_STR                      \
    "(ftp_telnet) Invalid FTP Command"
#define FTP_EO_PARAMETER_LENGTH_OVERFLOW_STR        \
    "(ftp_telnet) FTP command parameters were too long"
#define FTP_EO_MALFORMED_PARAMETER_STR              \
    "(ftp_telnet) FTP command parameters were malformed"
#define FTP_EO_PARAMETER_STR_FORMAT_STR             \
    "(ftp_telnet) FTP command parameters contained potential string format"
#define FTP_EO_RESPONSE_LENGTH_OVERFLOW_STR         \
    "(ftp_telnet) FTP response message was too long"
#define FTP_EO_ENCRYPTED_STR                        \
    "(ftp_telnet) FTP traffic encrypted"
#define FTP_EO_BOUNCE_STR                           \
    "(ftp_telnet) FTP bounce attempt"
#define FTP_EO_EVASIVE_TELNET_CMD_STR               \
    "(ftp_telnet) Evasive (incomplete) TELNET CMD on FTP Command Channel"

/*
 * TELNET Events
 */
#define TELNET_EO_AYT_OVERFLOW              0
#define TELNET_EO_ENCRYPTED                 1
#define TELNET_EO_SB_NO_SE                  2

#define TELNET_EO_AYT_OVERFLOW_SID              1
#define TELNET_EO_ENCRYPTED_SID                 2
#define TELNET_EO_SB_NO_SE_SID                  3


/*
 * IMPORTANT:
 * Every time you add a telnet event, this number must be
 * incremented.
 */
#define TELNET_EO_EVENT_NUM      3

/*
 * These defines are the alert names for each event
 */
#define TELNET_EO_AYT_OVERFLOW_STR                  \
    "(ftp_telnet) Consecutive Telnet AYT commands beyond threshold"
#define TELNET_EO_ENCRYPTED_STR                     \
    "(ftp_telnet) Telnet traffic encrypted"
#define TELNET_EO_SB_NO_SE_STR                      \
    "(ftp_telnet) Telnet Subnegotiation Begin Command without Subnegotiation End"

/*
 * Event Priorities
 */
#define FTPP_EO_HIGH_PRIORITY 0
#define FTPP_EO_MED_PRIORITY  1
#define FTPP_EO_LOW_PRIORITY  2

#endif
