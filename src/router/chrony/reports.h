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
  enum {
    RPT_NONSELECTABLE,
    RPT_FALSETICKER,
    RPT_JITTERY,
    RPT_SELECTABLE,
    RPT_UNSELECTED,
    RPT_SELECTED,
  } state;

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
  unsigned long n_samples;
  unsigned long n_runs;
  unsigned long span_seconds;
  double rtc_seconds_fast;
  double rtc_gain_rate_ppm;
} RPT_RTC_Report;

typedef struct {
  IPAddr ip_addr;
  uint32_t ntp_hits;
  uint32_t nke_hits;
  uint32_t cmd_hits;
  uint16_t ntp_drops;
  uint16_t nke_drops;
  uint16_t cmd_drops;
  int8_t ntp_interval;
  int8_t nke_interval;
  int8_t cmd_interval;
  int8_t ntp_timeout_interval;
  uint32_t last_ntp_hit_ago;
  uint32_t last_nke_hit_ago;
  uint32_t last_cmd_hit_ago;
} RPT_ClientAccessByIndex_Report;

typedef struct {
  uint64_t ntp_hits;
  uint64_t nke_hits;
  uint64_t cmd_hits;
  uint64_t ntp_drops;
  uint64_t nke_drops;
  uint64_t cmd_drops;
  uint64_t log_drops;
  uint64_t ntp_auth_hits;
  uint64_t ntp_interleaved_hits;
  uint64_t ntp_timestamps;
  uint64_t ntp_span_seconds;
  uint64_t ntp_daemon_rx_timestamps;
  uint64_t ntp_daemon_tx_timestamps;
  uint64_t ntp_kernel_rx_timestamps;
  uint64_t ntp_kernel_tx_timestamps;
  uint64_t ntp_hw_rx_timestamps;
  uint64_t ntp_hw_tx_timestamps;
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
  uint32_t total_good_count;
  uint32_t total_kernel_tx_ts;
  uint32_t total_kernel_rx_ts;
  uint32_t total_hw_tx_ts;
  uint32_t total_hw_rx_ts;
} RPT_NTPReport;

typedef struct {
  NTP_AuthMode mode;
  uint32_t key_id;
  int key_type;
  int key_length;
  int ke_attempts;
  uint32_t last_ke_ago;
  int cookies;
  int cookie_length;
  int nak;
} RPT_AuthReport;

typedef struct {
  uint32_t ref_id;
  IPAddr ip_addr;
  char state_char;
  int authentication;
  NTP_Leap leap;
  int conf_options;
  int eff_options;
  uint32_t last_sample_ago;
  double score;
  double lo_limit;
  double hi_limit;
} RPT_SelectReport;

#endif /* GOT_REPORTS_H */
