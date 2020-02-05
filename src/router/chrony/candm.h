/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
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

  Definitions for the network protocol used for command and monitoring
  of the timeserver.

  */

#ifndef GOT_CANDM_H
#define GOT_CANDM_H

#include "sysincl.h"
#include "addressing.h"

/* This is the default port to use for CANDM, if no alternative is
   defined */
#define DEFAULT_CANDM_PORT 323

/* Request codes */
#define REQ_NULL 0
#define REQ_ONLINE 1
#define REQ_OFFLINE 2
#define REQ_BURST 3
#define REQ_MODIFY_MINPOLL 4
#define REQ_MODIFY_MAXPOLL 5
#define REQ_DUMP 6
#define REQ_MODIFY_MAXDELAY 7
#define REQ_MODIFY_MAXDELAYRATIO 8
#define REQ_MODIFY_MAXUPDATESKEW 9
#define REQ_LOGON 10
#define REQ_SETTIME 11
#define REQ_LOCAL 12
#define REQ_MANUAL 13
#define REQ_N_SOURCES 14
#define REQ_SOURCE_DATA 15
#define REQ_REKEY 16
#define REQ_ALLOW 17
#define REQ_ALLOWALL 18
#define REQ_DENY 19
#define REQ_DENYALL 20
#define REQ_CMDALLOW 21
#define REQ_CMDALLOWALL 22
#define REQ_CMDDENY 23
#define REQ_CMDDENYALL 24
#define REQ_ACCHECK 25
#define REQ_CMDACCHECK 26
#define REQ_ADD_SERVER 27
#define REQ_ADD_PEER 28
#define REQ_DEL_SOURCE 29
#define REQ_WRITERTC 30
#define REQ_DFREQ 31
#define REQ_DOFFSET 32
#define REQ_TRACKING 33
#define REQ_SOURCESTATS 34
#define REQ_RTCREPORT 35
#define REQ_TRIMRTC 36
#define REQ_CYCLELOGS 37
#define REQ_SUBNETS_ACCESSED 38
#define REQ_CLIENT_ACCESSES 39
#define REQ_CLIENT_ACCESSES_BY_INDEX 40
#define REQ_MANUAL_LIST 41
#define REQ_MANUAL_DELETE 42
#define REQ_MAKESTEP 43
#define REQ_ACTIVITY 44
#define REQ_MODIFY_MINSTRATUM 45
#define REQ_MODIFY_POLLTARGET 46
#define REQ_MODIFY_MAXDELAYDEVRATIO 47
#define REQ_RESELECT 48
#define REQ_RESELECTDISTANCE 49
#define REQ_MODIFY_MAKESTEP 50
#define REQ_SMOOTHING 51
#define REQ_SMOOTHTIME 52
#define REQ_REFRESH 53
#define REQ_SERVER_STATS 54
#define REQ_CLIENT_ACCESSES_BY_INDEX2 55
#define REQ_LOCAL2 56
#define REQ_NTP_DATA 57
#define REQ_ADD_SERVER2 58
#define REQ_ADD_PEER2 59
#define REQ_ADD_SERVER3 60
#define REQ_ADD_PEER3 61
#define REQ_SHUTDOWN 62
#define REQ_ONOFFLINE 63
#define N_REQUEST_TYPES 64

/* Structure used to exchange timespecs independent of time_t size */
typedef struct {
  uint32_t tv_sec_high;
  uint32_t tv_sec_low;
  uint32_t tv_nsec;
} Timespec;

/* This is used in tv_sec_high for 32-bit timestamps */
#define TV_NOHIGHSEC 0x7fffffff

/* 32-bit floating-point format consisting of 7-bit signed exponent
   and 25-bit signed coefficient without hidden bit.
   The result is calculated as: 2^(exp - 25) * coef */
typedef struct {
  int32_t f;
} Float;

/* The EOR (end of record) fields are used by the offsetof operator in
   pktlength.c, to get the number of bytes that ought to be
   transmitted for each packet type. */

typedef struct {
  int32_t EOR;
} REQ_Null;

typedef struct {
  IPAddr mask;
  IPAddr address;
  int32_t EOR;
} REQ_Online;

typedef struct {
  IPAddr mask;
  IPAddr address;
  int32_t EOR;
} REQ_Offline;

typedef struct {
  IPAddr mask;
  IPAddr address;
  int32_t n_good_samples;
  int32_t n_total_samples;
  int32_t EOR;
} REQ_Burst;

typedef struct {
  IPAddr address;
  int32_t new_minpoll;
  int32_t EOR;
} REQ_Modify_Minpoll;

typedef struct {
  IPAddr address;
  int32_t new_maxpoll;
  int32_t EOR;
} REQ_Modify_Maxpoll;

typedef struct {
  int32_t pad;
  int32_t EOR;
} REQ_Dump;

typedef struct {
  IPAddr address;
  Float new_max_delay;
  int32_t EOR;
} REQ_Modify_Maxdelay;

typedef struct {
  IPAddr address;
  Float new_max_delay_ratio;
  int32_t EOR;
} REQ_Modify_Maxdelayratio;

typedef struct {
  IPAddr address;
  Float new_max_delay_dev_ratio;
  int32_t EOR;
} REQ_Modify_Maxdelaydevratio;

typedef struct {
  IPAddr address;
  int32_t new_min_stratum;
  int32_t EOR;
} REQ_Modify_Minstratum;

typedef struct {
  IPAddr address;
  int32_t new_poll_target;
  int32_t EOR;
} REQ_Modify_Polltarget;

typedef struct {
  Float new_max_update_skew;
  int32_t EOR;
} REQ_Modify_Maxupdateskew;

typedef struct {
  int32_t limit;
  Float threshold;
  int32_t EOR;
} REQ_Modify_Makestep;

typedef struct {
  Timespec ts;
  int32_t EOR;
} REQ_Logon;

typedef struct {
  Timespec ts;
  int32_t EOR;
} REQ_Settime;

typedef struct {
  int32_t on_off;
  int32_t stratum;
  Float distance;
  int32_t orphan;
  int32_t EOR;
} REQ_Local;

typedef struct {
  int32_t option;
  int32_t EOR;
} REQ_Manual;

typedef struct {
  int32_t index;
  int32_t EOR;
} REQ_Source_Data;

typedef struct {
  IPAddr ip;
  int32_t subnet_bits;
  int32_t EOR;
} REQ_Allow_Deny;

typedef struct {
  IPAddr ip;
  int32_t EOR;
} REQ_Ac_Check;

/* Flags used in NTP source requests */
#define REQ_ADDSRC_ONLINE 0x1
#define REQ_ADDSRC_AUTOOFFLINE 0x2
#define REQ_ADDSRC_IBURST 0x4
#define REQ_ADDSRC_PREFER 0x8
#define REQ_ADDSRC_NOSELECT 0x10
#define REQ_ADDSRC_TRUST 0x20
#define REQ_ADDSRC_REQUIRE 0x40
#define REQ_ADDSRC_INTERLEAVED 0x80
#define REQ_ADDSRC_BURST 0x100

typedef struct {
  IPAddr ip_addr;
  uint32_t port;
  int32_t minpoll;
  int32_t maxpoll;
  int32_t presend_minpoll;
  uint32_t min_stratum;
  uint32_t poll_target;
  uint32_t version;
  uint32_t max_sources;
  int32_t min_samples;
  int32_t max_samples;
  uint32_t authkey;
  Float max_delay;
  Float max_delay_ratio;
  Float max_delay_dev_ratio;
  Float min_delay;
  Float asymmetry;
  Float offset;
  uint32_t flags;
  int32_t filter_length;
  uint32_t reserved[3];
  int32_t EOR;
} REQ_NTP_Source;

typedef struct {
  IPAddr ip_addr;
  int32_t EOR;
} REQ_Del_Source;

typedef struct {
  Float dfreq;
  int32_t EOR;
} REQ_Dfreq;

typedef struct {
  int32_t sec;
  int32_t usec;
  int32_t EOR;
} REQ_Doffset;

typedef struct {
  uint32_t index;
  int32_t EOR;
} REQ_Sourcestats;

/* This is based on the response size rather than the
   request size */
#define MAX_CLIENT_ACCESSES 8

typedef struct {
  uint32_t first_index;
  uint32_t n_clients;
  int32_t EOR;
} REQ_ClientAccessesByIndex;

typedef struct {
  int32_t index;
  int32_t EOR;
} REQ_ManualDelete;

typedef struct {
  Float distance;
  int32_t EOR;
} REQ_ReselectDistance;

#define REQ_SMOOTHTIME_RESET 0
#define REQ_SMOOTHTIME_ACTIVATE 1

typedef struct {
  int32_t option;
  int32_t EOR;
} REQ_SmoothTime;

typedef struct {
  IPAddr ip_addr;
  int32_t EOR;
} REQ_NTPData;

/* ================================================== */

#define PKT_TYPE_CMD_REQUEST 1
#define PKT_TYPE_CMD_REPLY 2

/* This version number needs to be incremented whenever the packet
   size and/or the format of any of the existing messages is changed.
   Other changes, e.g. new command types, should be handled cleanly by
   client.c and cmdmon.c anyway, so the version can stay the same.
   
   Version 1 : original version with fixed size packets

   Version 2 : both command and reply packet sizes made capable of
   being variable length.

   Version 3 : NTP_Source message lengthened (auto_offline)

   Version 4 : IPv6 addressing added, 64-bit time values, sourcestats 
   and tracking reports extended, added flags to NTP source request,
   trimmed source report, replaced fixed-point format with floating-point
   and used also instead of integer microseconds, new commands: modify stratum,
   modify polltarget, modify maxdelaydevratio, reselect, reselectdistance

   Version 5 : auth data moved to the end of the packet to allow hashes with
   different sizes, extended sources, tracking and activity reports, dropped
   subnets accessed and client accesses

   Version 6 : added padding to requests to prevent amplification attack,
   changed maximum number of samples in manual list to 16, new commands: modify
   makestep, smoothing, smoothtime

   Support for authentication was removed later in version 6 of the protocol
   and commands that required authentication are allowed only locally over Unix
   domain socket.

   Version 6 (no authentication) : changed format of client accesses by index
   (using new request/reply types) and manual timestamp, added new fields and
   flags to NTP source request and report, made length of manual list constant,
   added new commands: ntpdata, refresh, serverstats, shutdown
 */

#define PROTO_VERSION_NUMBER 6

/* The oldest protocol versions that are compatible enough with the current
   version to report a version mismatch for the server and the client */
#define PROTO_VERSION_MISMATCH_COMPAT_SERVER 5
#define PROTO_VERSION_MISMATCH_COMPAT_CLIENT 4

/* The first protocol version using padding in requests */
#define PROTO_VERSION_PADDING 6

/* The maximum length of padding in request packet, currently
   defined by MANUAL_LIST */
#define MAX_PADDING_LENGTH 396

/* ================================================== */

typedef struct {
  uint8_t version; /* Protocol version */
  uint8_t pkt_type; /* What sort of packet this is */
  uint8_t res1;
  uint8_t res2;
  uint16_t command; /* Which command is being issued */
  uint16_t attempt; /* How many resends the client has done
                             (count up from zero for same sequence
                             number) */
  uint32_t sequence; /* Client's sequence number */
  uint32_t pad1;
  uint32_t pad2;

  union {
    REQ_Null null;
    REQ_Online online;
    REQ_Offline offline;
    REQ_Burst burst;
    REQ_Modify_Minpoll modify_minpoll;
    REQ_Modify_Maxpoll modify_maxpoll;
    REQ_Dump dump;
    REQ_Modify_Maxdelay modify_maxdelay;
    REQ_Modify_Maxdelayratio modify_maxdelayratio;
    REQ_Modify_Maxdelaydevratio modify_maxdelaydevratio;
    REQ_Modify_Minstratum modify_minstratum;
    REQ_Modify_Polltarget modify_polltarget;
    REQ_Modify_Maxupdateskew modify_maxupdateskew;
    REQ_Modify_Makestep modify_makestep;
    REQ_Logon logon;
    REQ_Settime settime;
    REQ_Local local;
    REQ_Manual manual;
    REQ_Source_Data source_data;
    REQ_Allow_Deny allow_deny;
    REQ_Ac_Check ac_check;
    REQ_NTP_Source ntp_source;
    REQ_Del_Source del_source;
    REQ_Dfreq dfreq;
    REQ_Doffset doffset;
    REQ_Sourcestats sourcestats;
    REQ_ClientAccessesByIndex client_accesses_by_index;
    REQ_ManualDelete manual_delete;
    REQ_ReselectDistance reselect_distance;
    REQ_SmoothTime smoothtime;
    REQ_NTPData ntp_data;
  } data; /* Command specific parameters */

  /* Padding used to prevent traffic amplification.  It only defines the
     maximum size of the packet, there is no hole after the data field. */
  uint8_t padding[MAX_PADDING_LENGTH];

} CMD_Request;

/* ================================================== */
/* Authority codes for command types */

#define PERMIT_OPEN 0
#define PERMIT_LOCAL 1
#define PERMIT_AUTH 2

/* ================================================== */

/* Reply codes */
#define RPY_NULL 1
#define RPY_N_SOURCES 2
#define RPY_SOURCE_DATA 3
#define RPY_MANUAL_TIMESTAMP 4
#define RPY_TRACKING 5
#define RPY_SOURCESTATS 6
#define RPY_RTC 7
#define RPY_SUBNETS_ACCESSED 8
#define RPY_CLIENT_ACCESSES 9
#define RPY_CLIENT_ACCESSES_BY_INDEX 10
#define RPY_MANUAL_LIST 11
#define RPY_ACTIVITY 12
#define RPY_SMOOTHING 13
#define RPY_SERVER_STATS 14
#define RPY_CLIENT_ACCESSES_BY_INDEX2 15
#define RPY_NTP_DATA 16
#define RPY_MANUAL_TIMESTAMP2 17
#define RPY_MANUAL_LIST2 18
#define N_REPLY_TYPES 19

/* Status codes */
#define STT_SUCCESS 0
#define STT_FAILED 1
#define STT_UNAUTH 2
#define STT_INVALID 3
#define STT_NOSUCHSOURCE 4
#define STT_INVALIDTS 5
#define STT_NOTENABLED 6
#define STT_BADSUBNET 7
#define STT_ACCESSALLOWED 8
#define STT_ACCESSDENIED 9
/* Deprecated */
#define STT_NOHOSTACCESS 10
#define STT_SOURCEALREADYKNOWN 11
#define STT_TOOMANYSOURCES 12
#define STT_NORTC 13
#define STT_BADRTCFILE 14
#define STT_INACTIVE 15
#define STT_BADSAMPLE 16
#define STT_INVALIDAF 17
#define STT_BADPKTVERSION 18
#define STT_BADPKTLENGTH 19

typedef struct {
  int32_t EOR;
} RPY_Null;

typedef struct {
  uint32_t n_sources;
  int32_t EOR;
} RPY_N_Sources;

#define RPY_SD_MD_CLIENT 0
#define RPY_SD_MD_PEER   1
#define RPY_SD_MD_REF    2

#define RPY_SD_ST_SYNC 0
#define RPY_SD_ST_UNREACH 1
#define RPY_SD_ST_FALSETICKER 2
#define RPY_SD_ST_JITTERY 3
#define RPY_SD_ST_CANDIDATE 4
#define RPY_SD_ST_OUTLIER 5

#define RPY_SD_FLAG_NOSELECT 0x1
#define RPY_SD_FLAG_PREFER 0x2
#define RPY_SD_FLAG_TRUST 0x4
#define RPY_SD_FLAG_REQUIRE 0x8

typedef struct {
  IPAddr ip_addr;
  int16_t poll;
  uint16_t stratum;
  uint16_t state;
  uint16_t mode;
  uint16_t flags;
  uint16_t reachability;
  uint32_t  since_sample;
  Float orig_latest_meas;
  Float latest_meas;
  Float latest_meas_err;
  int32_t EOR;
} RPY_Source_Data;

typedef struct {
  uint32_t ref_id;
  IPAddr ip_addr;
  uint16_t stratum;
  uint16_t leap_status;
  Timespec ref_time;
  Float current_correction;
  Float last_offset;
  Float rms_offset;
  Float freq_ppm;
  Float resid_freq_ppm;
  Float skew_ppm;
  Float root_delay;
  Float root_dispersion;
  Float last_update_interval;
  int32_t EOR;
} RPY_Tracking;

typedef struct {
  uint32_t ref_id;
  IPAddr ip_addr;
  uint32_t n_samples;
  uint32_t n_runs;
  uint32_t span_seconds;
  Float sd;
  Float resid_freq_ppm;
  Float skew_ppm;
  Float est_offset;
  Float est_offset_err;
  int32_t EOR;
} RPY_Sourcestats;

typedef struct {
  Timespec ref_time;
  uint16_t n_samples;
  uint16_t n_runs;
  uint32_t span_seconds;
  Float rtc_seconds_fast;
  Float rtc_gain_rate_ppm;
  int32_t EOR;
} RPY_Rtc;

typedef struct {
  Float offset;
  Float dfreq_ppm;
  Float new_afreq_ppm;
  int32_t EOR;
} RPY_ManualTimestamp;

typedef struct {
  IPAddr ip;
  uint32_t ntp_hits;
  uint32_t cmd_hits;
  uint32_t ntp_drops;
  uint32_t cmd_drops;
  int8_t ntp_interval;
  int8_t cmd_interval;
  int8_t ntp_timeout_interval;
  int8_t pad;
  uint32_t last_ntp_hit_ago;
  uint32_t last_cmd_hit_ago;
} RPY_ClientAccesses_Client;

typedef struct {
  uint32_t n_indices;      /* how many indices there are in the server's table */
  uint32_t next_index;     /* the index 1 beyond those processed on this call */
  uint32_t n_clients;      /* the number of valid entries in the following array */
  RPY_ClientAccesses_Client clients[MAX_CLIENT_ACCESSES];
  int32_t EOR;
} RPY_ClientAccessesByIndex;

typedef struct {
  uint32_t ntp_hits;
  uint32_t cmd_hits;
  uint32_t ntp_drops;
  uint32_t cmd_drops;
  uint32_t log_drops;
  int32_t EOR;
} RPY_ServerStats;

#define MAX_MANUAL_LIST_SAMPLES 16

typedef struct {
  Timespec when;
  Float slewed_offset;
  Float orig_offset;
  Float residual;
} RPY_ManualListSample;

typedef struct {
  uint32_t n_samples;
  RPY_ManualListSample samples[MAX_MANUAL_LIST_SAMPLES];
  int32_t EOR;
} RPY_ManualList;

typedef struct {
  int32_t online;
  int32_t offline;
  int32_t burst_online;
  int32_t burst_offline;
  int32_t unresolved;
  int32_t EOR;
} RPY_Activity;

#define RPY_SMT_FLAG_ACTIVE 0x1
#define RPY_SMT_FLAG_LEAPONLY 0x2

typedef struct {
  uint32_t flags;
  Float offset;
  Float freq_ppm;
  Float wander_ppm;
  Float last_update_ago;
  Float remaining_time;
  int32_t EOR;
} RPY_Smoothing;

#define RPY_NTP_FLAGS_TESTS 0x3ff
#define RPY_NTP_FLAG_INTERLEAVED 0x4000
#define RPY_NTP_FLAG_AUTHENTICATED 0x8000

typedef struct {
  IPAddr remote_addr;
  IPAddr local_addr;
  uint16_t remote_port;
  uint8_t leap;
  uint8_t version;
  uint8_t mode;
  uint8_t stratum;
  int8_t poll;
  int8_t precision;
  Float root_delay;
  Float root_dispersion;
  uint32_t ref_id;
  Timespec ref_time;
  Float offset;
  Float peer_delay;
  Float peer_dispersion;
  Float response_time;
  Float jitter_asymmetry;
  uint16_t flags;
  uint8_t tx_tss_char;
  uint8_t rx_tss_char;
  uint32_t total_tx_count;
  uint32_t total_rx_count;
  uint32_t total_valid_count;
  uint32_t reserved[4];
  int32_t EOR;
} RPY_NTPData;

typedef struct {
  uint8_t version;
  uint8_t pkt_type;
  uint8_t res1;
  uint8_t res2;
  uint16_t command; /* Which command is being replied to */
  uint16_t reply; /* Which format of reply this is */
  uint16_t status; /* Status of command processing */
  uint16_t pad1; /* Padding for compatibility and 4 byte alignment */
  uint16_t pad2;
  uint16_t pad3;
  uint32_t sequence; /* Echo of client's sequence number */
  uint32_t pad4;
  uint32_t pad5;

  union {
    RPY_Null null;
    RPY_N_Sources n_sources;
    RPY_Source_Data source_data;
    RPY_ManualTimestamp manual_timestamp;
    RPY_Tracking tracking;
    RPY_Sourcestats sourcestats;
    RPY_Rtc rtc;
    RPY_ClientAccessesByIndex client_accesses_by_index;
    RPY_ServerStats server_stats;
    RPY_ManualList manual_list;
    RPY_Activity activity;
    RPY_Smoothing smoothing;
    RPY_NTPData ntp_data;
  } data; /* Reply specific parameters */

} CMD_Reply;

/* ================================================== */

#endif /* GOT_CANDM_H */
