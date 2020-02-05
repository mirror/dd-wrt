/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2002
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

  Data structure definitions within the daemon for various reports that
  can be generated */

#ifndef GOT_REPORTS_H
#define GOT_REPORTS_H

#include "sysincl.h"
#include "addressing.h"
#include "ntp.h"

typedef struct {
  IPAddr ip_addr;
  int stratum;
  int poll;
  enum {RPT_NTP_CLIENT, RPT_NTP_PEER, RPT_LOCAL_REFERENCE} mode;
  enum {RPT_SYNC, RPT_UNREACH, RPT_FALSETICKER, RPT_JITTERY, RPT_CANDIDATE, RPT_OUTLIER} state;
  int sel_options;

  int reachability;
  unsigned long latest_meas_ago; /* seconds */
  double orig_latest_meas; /* seconds */
  double latest_meas; /* seconds */
  double latest_meas_err; /* seconds */
} RPT_SourceReport ;

typedef struct {
  uint32_t ref_id;
  IPAddr ip_addr;
  int stratum;
  NTP_Leap leap_status;
  struct timespec ref_time;
  double current_correction;
  double last_offset;
  double rms_offset;
  double freq_ppm;
  double resid_freq_ppm;
  double skew_ppm;
  double root_delay;
  double root_dispersion;
  double last_update_interval;
} RPT_TrackingReport;

typedef struct {
  uint32_t ref_id;
  IPAddr ip_addr;
  unsigned long n_samples;
  unsigned long n_runs;
  unsigned long span_seconds;
  double resid_freq_ppm;
  double skew_ppm;
  double sd;
  double est_offset;
  double est_offset_err;
} RPT_SourcestatsReport;

typedef struct {
  struct timespec ref_time;
  unsigned short n_samples;
  unsigned short n_runs;
  unsigned long span_seconds;
  double rtc_seconds_fast;
  double rtc_gain_rate_ppm;
} RPT_RTC_Report;

typedef struct {
  IPAddr ip_addr;
  uint32_t ntp_hits;
  uint32_t cmd_hits;
  uint16_t ntp_drops;
  uint16_t cmd_drops;
  int8_t ntp_interval;
  int8_t cmd_interval;
  int8_t ntp_timeout_interval;
  uint32_t last_ntp_hit_ago;
  uint32_t last_cmd_hit_ago;
} RPT_ClientAccessByIndex_Report;

typedef struct {
  uint32_t ntp_hits;
  uint32_t cmd_hits;
  uint32_t ntp_drops;
  uint32_t cmd_drops;
  uint32_t log_drops;
} RPT_ServerStatsReport;

typedef struct {
  struct timespec when;
  double slewed_offset;
  double orig_offset;
  double residual;
} RPT_ManualSamplesReport;

typedef struct {
  int online;
  int offline;
  int burst_online;
  int burst_offline;
  int unresolved;
} RPT_ActivityReport;

typedef struct {
  int active;
  int leap_only;
  double offset;
  double freq_ppm;
  double wander_ppm;
  double last_update_ago;
  double remaining_time;
} RPT_SmoothingReport;

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
  double root_delay;
  double root_dispersion;
  uint32_t ref_id;
  struct timespec ref_time;
  double offset;
  double peer_delay;
  double peer_dispersion;
  double response_time;
  double jitter_asymmetry;
  uint16_t tests;
  int interleaved;
  int authenticated;
  char tx_tss_char;
  char rx_tss_char;
  uint32_t total_tx_count;
  uint32_t total_rx_count;
  uint32_t total_valid_count;
} RPT_NTPReport;

#endif /* GOT_REPORTS_H */
