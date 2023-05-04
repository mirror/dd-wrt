/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
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

#ifndef _PREPROC_IDS_H
#define _PREPROC_IDS_H

#include <stdint.h>
#ifdef DUMP_BUFFER
#include "sf_types.h"
#endif
/*
**  Preprocessor Communication Defines
**  ----------------------------------
**  These defines allow preprocessors to be turned
**  on and off for each packet.  Preprocessors can be
**  turned off and on before preprocessing occurs and
**  during preprocessing.
**
**  Currently, the order in which the preprocessors are
**  placed in the snort.conf determine the order of
**  evaluation.  So if one module wants to turn off
**  another module, it must come first in the order.
*/

// currently 64 bits (preprocessors)
// are available.

#define PP_BO                      0
#define PP_APP_ID                  1
#define PP_DNS                     2
#define PP_FRAG3                   3
#define PP_FTPTELNET               4
#define PP_HTTPINSPECT             5
#define PP_PERFMONITOR             6
#define PP_RPCDECODE               7
#define PP_SHARED_RULES            8
#define PP_SFPORTSCAN              9
#define PP_SMTP                   10
#define PP_SSH                    11
#define PP_SSL                    12
#define PP_STREAM                 13
#define PP_TELNET                 14
#define PP_ARPSPOOF               15
#define PP_DCE2                   16
#define PP_SDF                    17
#define PP_NORMALIZE              18
#define PP_ISAKMP                 19  // used externally
#define PP_SESSION                20
#define PP_SIP                    21
#define PP_POP                    22
#define PP_IMAP                   23
#define PP_NETWORK_DISCOVERY      24  // used externally
#define PP_FW_RULE_ENGINE         25  // used externally
#define PP_REPUTATION             26
#define PP_GTP                    27
#define PP_MODBUS                 28
#define PP_DNP3                   29
#define PP_FILE                   30
#define PP_FILE_INSPECT           31
#define PP_NAP_RULE_ENGINE        32
#define PP_PREFILTER_RULE_ENGINE  33  // used externally
#define PP_HTTPMOD                34
#define PP_HTTP2                  35
#define PP_CIP                    36
#define PP_S7COMMPLUS             37
#define PP_MAX                    38
#define PP_ALL                    50
#define PP_ENABLE_ALL (~0)
#define PP_DISABLE_ALL 0x0

#ifdef WIN32
#ifndef UINT64_C
#define UINT64_C(v) (v)
#endif
#endif

// preprocessors that run before or as part of Network Analysis Policy processing... If enabled by
// configuration they are never disabled
#define PP_CLASS_NETWORK ( ( UINT64_C(1) << PP_FRAG3 ) | ( UINT64_C(1) << PP_PERFMONITOR ) | \
                           ( UINT64_C(1) << PP_SFPORTSCAN ) | ( UINT64_C(1) << PP_STREAM ) | \
                           ( UINT64_C(1) << PP_NORMALIZE ) | ( UINT64_C(1) << PP_SESSION ) | \
                           ( UINT64_C(1) << PP_REPUTATION ) )

// Firewall and Application ID & Netowrk Discovery preprocessors...also always run if enabled by configuration
#define PP_CLASS_NGFW ( ( UINT64_C(1) << PP_APP_ID ) | ( UINT64_C(1) << PP_FW_RULE_ENGINE ) | \
                        ( UINT64_C(1) << PP_NETWORK_DISCOVERY ) | ( UINT64_C(1) << PP_PREFILTER_RULE_ENGINE ) | \
                        ( UINT64_C(1) << PP_HTTPMOD) )

// Application preprocessors...once the application or protocol for a stream is determined only preprocessors
// that analyze that type of stream are enabled (usually there is only 1...)
#define PP_CLASS_PROTO_APP ( ( UINT64_C(1) << PP_BO ) | ( UINT64_C(1) << PP_DNS ) | \
                             ( UINT64_C(1) << PP_FTPTELNET ) | ( UINT64_C(1) << PP_HTTPINSPECT ) | \
                             ( UINT64_C(1) << PP_RPCDECODE ) | ( UINT64_C(1) << PP_SHARED_RULES ) | \
                             ( UINT64_C(1) << PP_SMTP ) | ( UINT64_C(1) << PP_SSH ) | \
                             ( UINT64_C(1) << PP_SSL ) | ( UINT64_C(1) << PP_TELNET ) | \
                             ( UINT64_C(1) << PP_ARPSPOOF ) | ( UINT64_C(1) << PP_DCE2 ) | \
                             ( UINT64_C(1) << PP_SDF ) | ( UINT64_C(1) << PP_ISAKMP) | \
                             ( UINT64_C(1) << PP_POP ) | ( UINT64_C(1) << PP_IMAP ) | \
                             ( UINT64_C(1) << PP_GTP ) | ( UINT64_C(1) << PP_MODBUS ) | \
                             ( UINT64_C(1) << PP_DNP3 ) | ( UINT64_C(1) << PP_FILE ) | \
                             ( UINT64_C(1) << PP_FILE_INSPECT ) )

#define PP_DEFINED_GLOBAL ( ( UINT64_C(1) << PP_APP_ID ) | ( UINT64_C(1) << PP_FW_RULE_ENGINE ) | \
                            ( UINT64_C(1) << PP_NETWORK_DISCOVERY ) | ( UINT64_C(1) << PP_PERFMONITOR) | \
                            ( UINT64_C(1) << PP_SESSION ) | ( UINT64_C(1) << PP_PREFILTER_RULE_ENGINE ) )

#define PP_CORE_ORDER_SESSION   0
#define PP_CORE_ORDER_IPREP     1
#define PP_CORE_ORDER_NAP       2
#define PP_CORE_ORDER_NORML     3
#define PP_CORE_ORDER_FRAG3     4
#define PP_CORE_ORDER_PREFILTER 5   // used externally
#define PP_CORE_ORDER_STREAM    6

#define PRIORITY_CORE            0x0
#define PRIORITY_CORE_LAST      0x0f
#define PRIORITY_FIRST          0x10
#define PRIORITY_NETWORK        0x20
#define PRIORITY_TRANSPORT     0x100
#define PRIORITY_TUNNEL        0x105
#define PRIORITY_SCANNER       0x110
#define PRIORITY_APPLICATION   0x200
#define PRIORITY_LAST         0xffff

#ifdef DUMP_BUFFER

/* dump_alert_only makes sure that bufferdump happens only when a rule is
   triggered.

   dumped_state avoids repeatition of buffer dump for a packet that has an
   alert, when --buffer-dump is given as command line option.

   dump_enabled gets set when --buffer-dump or --buffer-dump-alert option
   is given.
*/

extern bool dump_alert_only;
extern bool dumped_state;
extern bool dump_enabled;

#define MAX_BUFFER_DUMP_FUNC 13
#define MAX_HTTP_BUFFER_DUMP 16
#define MAX_SMTP_BUFFER_DUMP 7
#define MAX_SIP_BUFFER_DUMP 16
#define MAX_DNP3_BUFFER_DUMP 4
#define MAX_POP_BUFFER_DUMP 7
#define MAX_MODBUS_BUFFER_DUMP 3
#define MAX_SSH_BUFFER_DUMP 11
#define MAX_DNS_BUFFER_DUMP 10
#define MAX_DCERPC2_BUFFER_DUMP 7
#define MAX_FTPTELNET_BUFFER_DUMP 7
#define MAX_IMAP_BUFFER_DUMP 4
#define MAX_SSL_BUFFER_DUMP 4
#define MAX_GTP_BUFFER_DUMP 6

typedef enum {
    HTTP_BUFFER_DUMP_FUNC,
    SMTP_BUFFER_DUMP_FUNC,
    SIP_BUFFER_DUMP_FUNC,
    DNP3_BUFFER_DUMP_FUNC,
    POP_BUFFER_DUMP_FUNC,
    MODBUS_BUFFER_DUMP_FUNC,
    SSH_BUFFER_DUMP_FUNC,
    DNS_BUFFER_DUMP_FUNC,
    DCERPC2_BUFFER_DUMP_FUNC,
    FTPTELNET_BUFFER_DUMP_FUNC,
    IMAP_BUFFER_DUMP_FUNC,
    SSL_BUFFER_DUMP_FUNC,
    GTP_BUFFER_DUMP_FUNC
} BUFFER_DUMP_FUNC;

typedef struct _TraceBuffer {
    char *buf_name;
    char *buf_content;
    uint16_t length;
} TraceBuffer;

typedef uint64_t BufferDumpEnableMask;
extern TraceBuffer *(*getBuffers[MAX_BUFFER_DUMP_FUNC])(void);
extern BufferDumpEnableMask bdmask;

#endif

typedef uint64_t PreprocEnableMask;

#endif /* _PREPROC_IDS_H */

