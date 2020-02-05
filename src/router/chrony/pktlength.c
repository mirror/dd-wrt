/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2002
 * Copyright (C) Miroslav Lichvar  2014-2016
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 **********************************************************************

  =======================================================================

  Routines to compute the expected length of a command or reply packet.
  These operate on the RAW NETWORK packets, from the point of view of
  integer endianness within the structures.

  */
#include "config.h"

#include "sysincl.h"

#include "util.h"
#include "pktlength.h"

#define PADDING_LENGTH_(request_length, reply_length) \
  (uint16_t)((request_length) < (reply_length) ? (reply_length) - (request_length) : 0)

#define PADDING_LENGTH(request_data, reply_data) \
  PADDING_LENGTH_(offsetof(CMD_Request, request_data), offsetof(CMD_Reply, reply_data))

#define REQ_LENGTH_ENTRY(request_data_field, reply_data_field) \
  { offsetof(CMD_Request, data.request_data_field.EOR), \
    PADDING_LENGTH(data.request_data_field.EOR, data.reply_data_field.EOR) }

#define RPY_LENGTH_ENTRY(reply_data_field) \
  offsetof(CMD_Reply, data.reply_data_field.EOR)

/* ================================================== */

struct request_length {
  uint16_t command;
  uint16_t padding;
};

static const struct request_length request_lengths[] = {
  REQ_LENGTH_ENTRY(null, null),                 /* NULL */
  REQ_LENGTH_ENTRY(online, null),               /* ONLINE */
  REQ_LENGTH_ENTRY(offline, null),              /* OFFLINE */
  REQ_LENGTH_ENTRY(burst, null),                /* BURST */
  REQ_LENGTH_ENTRY(modify_minpoll, null),       /* MODIFY_MINPOLL */
  REQ_LENGTH_ENTRY(modify_maxpoll, null),       /* MODIFY_MAXPOLL */
  REQ_LENGTH_ENTRY(dump, null),                 /* DUMP */
  REQ_LENGTH_ENTRY(modify_maxdelay, null),      /* MODIFY_MAXDELAY */
  REQ_LENGTH_ENTRY(modify_maxdelayratio, null), /* MODIFY_MAXDELAYRATIO */
  REQ_LENGTH_ENTRY(modify_maxupdateskew, null), /* MODIFY_MAXUPDATESKEW */
  REQ_LENGTH_ENTRY(logon, null),                /* LOGON */
  REQ_LENGTH_ENTRY(settime, manual_timestamp),  /* SETTIME */
  { 0, 0 },                                     /* LOCAL */
  REQ_LENGTH_ENTRY(manual, null),               /* MANUAL */
  REQ_LENGTH_ENTRY(null, n_sources),            /* N_SOURCES */
  REQ_LENGTH_ENTRY(source_data, source_data),   /* SOURCE_DATA */
  REQ_LENGTH_ENTRY(null, null),                 /* REKEY */
  REQ_LENGTH_ENTRY(allow_deny, null),           /* ALLOW */
  REQ_LENGTH_ENTRY(allow_deny, null),           /* ALLOWALL */
  REQ_LENGTH_ENTRY(allow_deny, null),           /* DENY */
  REQ_LENGTH_ENTRY(allow_deny, null),           /* DENYALL */
  REQ_LENGTH_ENTRY(allow_deny, null),           /* CMDALLOW */
  REQ_LENGTH_ENTRY(allow_deny, null),           /* CMDALLOWALL */
  REQ_LENGTH_ENTRY(allow_deny, null),           /* CMDDENY */
  REQ_LENGTH_ENTRY(allow_deny, null),           /* CMDDENYALL */
  REQ_LENGTH_ENTRY(ac_check, null),             /* ACCHECK */
  REQ_LENGTH_ENTRY(ac_check, null),             /* CMDACCHECK */
  { 0, 0 },                                     /* ADD_SERVER */
  { 0, 0 },                                     /* ADD_PEER */
  REQ_LENGTH_ENTRY(del_source, null),           /* DEL_SOURCE */
  REQ_LENGTH_ENTRY(null, null),                 /* WRITERTC */
  REQ_LENGTH_ENTRY(dfreq, null),                /* DFREQ */
  REQ_LENGTH_ENTRY(doffset, null),              /* DOFFSET */
  REQ_LENGTH_ENTRY(null, tracking),             /* TRACKING */
  REQ_LENGTH_ENTRY(sourcestats, sourcestats),   /* SOURCESTATS */
  REQ_LENGTH_ENTRY(null, rtc),                  /* RTCREPORT */
  REQ_LENGTH_ENTRY(null, null),                 /* TRIMRTC */
  REQ_LENGTH_ENTRY(null, null),                 /* CYCLELOGS */
  { 0, 0 },                                     /* SUBNETS_ACCESSED - not supported */
  { 0, 0 },                                     /* CLIENT_ACCESSES - not supported */
  { 0, 0 },                                     /* CLIENT_ACCESSES_BY_INDEX - not supported */
  REQ_LENGTH_ENTRY(null, manual_list),          /* MANUAL_LIST */
  REQ_LENGTH_ENTRY(manual_delete, null),        /* MANUAL_DELETE */
  REQ_LENGTH_ENTRY(null, null),                 /* MAKESTEP */
  REQ_LENGTH_ENTRY(null, activity),             /* ACTIVITY */
  REQ_LENGTH_ENTRY(modify_minstratum, null),    /* MODIFY_MINSTRATUM */
  REQ_LENGTH_ENTRY(modify_polltarget, null),    /* MODIFY_POLLTARGET */
  REQ_LENGTH_ENTRY(modify_maxdelaydevratio, null), /* MODIFY_MAXDELAYDEVRATIO */
  REQ_LENGTH_ENTRY(null, null),                 /* RESELECT */
  REQ_LENGTH_ENTRY(reselect_distance, null),    /* RESELECTDISTANCE */
  REQ_LENGTH_ENTRY(modify_makestep, null),      /* MODIFY_MAKESTEP */
  REQ_LENGTH_ENTRY(null, smoothing),            /* SMOOTHING */
  REQ_LENGTH_ENTRY(smoothtime, null),           /* SMOOTHTIME */
  REQ_LENGTH_ENTRY(null, null),                 /* REFRESH */
  REQ_LENGTH_ENTRY(null, server_stats),         /* SERVER_STATS */
  REQ_LENGTH_ENTRY(client_accesses_by_index,
                   client_accesses_by_index),   /* CLIENT_ACCESSES_BY_INDEX2 */
  REQ_LENGTH_ENTRY(local, null),                /* LOCAL2 */
  REQ_LENGTH_ENTRY(ntp_data, ntp_data),         /* NTP_DATA */
  { 0, 0 },                                     /* ADD_SERVER2 */
  { 0, 0 },                                     /* ADD_PEER2 */
  REQ_LENGTH_ENTRY(ntp_source, null),           /* ADD_SERVER3 */
  REQ_LENGTH_ENTRY(ntp_source, null),           /* ADD_PEER3 */
  REQ_LENGTH_ENTRY(null, null),                 /* SHUTDOWN */
  REQ_LENGTH_ENTRY(null, null),                 /* ONOFFLINE */
};

static const uint16_t reply_lengths[] = {
  0,                                            /* empty slot */
  RPY_LENGTH_ENTRY(null),                       /* NULL */
  RPY_LENGTH_ENTRY(n_sources),                  /* N_SOURCES */
  RPY_LENGTH_ENTRY(source_data),                /* SOURCE_DATA */
  0,                                            /* MANUAL_TIMESTAMP */
  RPY_LENGTH_ENTRY(tracking),                   /* TRACKING */
  RPY_LENGTH_ENTRY(sourcestats),                /* SOURCESTATS */
  RPY_LENGTH_ENTRY(rtc),                        /* RTC */
  0,                                            /* SUBNETS_ACCESSED - not supported */
  0,                                            /* CLIENT_ACCESSES - not supported */
  0,                                            /* CLIENT_ACCESSES_BY_INDEX - not supported */
  0,                                            /* MANUAL_LIST - not supported */
  RPY_LENGTH_ENTRY(activity),                   /* ACTIVITY */
  RPY_LENGTH_ENTRY(smoothing),                  /* SMOOTHING */
  RPY_LENGTH_ENTRY(server_stats),               /* SERVER_STATS */
  RPY_LENGTH_ENTRY(client_accesses_by_index),   /* CLIENT_ACCESSES_BY_INDEX2 */
  RPY_LENGTH_ENTRY(ntp_data),                   /* NTP_DATA */
  RPY_LENGTH_ENTRY(manual_timestamp),           /* MANUAL_TIMESTAMP2 */
  RPY_LENGTH_ENTRY(manual_list),                /* MANUAL_LIST2 */
};

/* ================================================== */

int
PKL_CommandLength(CMD_Request *r)
{
  uint32_t type;
  int command_length;

  assert(sizeof (request_lengths) / sizeof (request_lengths[0]) == N_REQUEST_TYPES);

  type = ntohs(r->command);
  if (type >= N_REQUEST_TYPES)
    return 0;

  command_length = request_lengths[type].command;
  if (!command_length)
    return 0;

  return command_length + PKL_CommandPaddingLength(r);
}

/* ================================================== */

int
PKL_CommandPaddingLength(CMD_Request *r)
{
  uint32_t type;

  if (r->version < PROTO_VERSION_PADDING)
    return 0;

  type = ntohs(r->command);

  if (type >= N_REQUEST_TYPES)
    return 0;

  return request_lengths[ntohs(r->command)].padding;
}

/* ================================================== */

int
PKL_ReplyLength(CMD_Reply *r)
{
  uint32_t type;

  assert(sizeof (reply_lengths) / sizeof (reply_lengths[0]) == N_REPLY_TYPES);

  type = ntohs(r->reply);

  /* Note that reply type codes start from 1, not 0 */
  if (type < 1 || type >= N_REPLY_TYPES)
    return 0;

  return reply_lengths[type];
}

/* ================================================== */

