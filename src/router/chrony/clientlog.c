/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * Copyright (C) Miroslav Lichvar  2009, 2015-2017
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

  This module keeps a count of the number of successful accesses by
  clients, and the times of the last accesses.

  This can be used for status reporting, and (in the case of a
  server), if it needs to know which clients have made use of its data
  recently.

  */

#include "config.h"

#include "sysincl.h"

#include "array.h"
#include "clientlog.h"
#include "conf.h"
#include "memory.h"
#include "ntp.h"
#include "reports.h"
#include "util.h"
#include "logging.h"

typedef struct {
  IPAddr ip_addr;
  uint32_t last_ntp_hit;
  uint32_t last_cmd_hit;
  uint32_t ntp_hits;
  uint32_t cmd_hits;
  uint16_t ntp_drops;
  uint16_t cmd_drops;
  uint16_t ntp_tokens;
  uint16_t cmd_tokens;
  int8_t ntp_rate;
  int8_t cmd_rate;
  int8_t ntp_timeout_rate;
  uint8_t flags;
  NTP_int64 ntp_rx_ts;
  NTP_int64 ntp_tx_ts;
} Record;

/* Hash table of records, there is a fixed number of records per slot */
static ARR_Instance records;

#define SLOT_BITS 4

/* Number of records in one slot of the hash table */
#define SLOT_SIZE (1U << SLOT_BITS)

/* Minimum number of slots */
#define MIN_SLOTS 1

/* Maximum number of slots, this is a hard limit */
#define MAX_SLOTS (1U << (24 - SLOT_BITS))

/* Number of slots in the hash table */
static unsigned int slots;

/* Maximum number of slots given memory allocation limit */
static unsigned int max_slots;

/* Times of last hits are saved as 32-bit fixed point values */
#define TS_FRAC 4
#define INVALID_TS 0

/* Static offset included in conversion to the fixed-point timestamps to
   randomise their alignment */
static uint32_t ts_offset;

/* Request rates are saved in the record as 8-bit scaled log2 values */
#define RATE_SCALE 4
#define MIN_RATE (-14 * RATE_SCALE)
#define INVALID_RATE -128

/* Response rates are controlled by token buckets.  The capacity and
   number of tokens spent on response are determined from configured
   minimum inverval between responses (in log2) and burst length. */

#define MIN_LIMIT_INTERVAL (-15 - TS_FRAC)
#define MAX_LIMIT_INTERVAL 12
#define MIN_LIMIT_BURST 1
#define MAX_LIMIT_BURST 255

static uint16_t max_ntp_tokens;
static uint16_t max_cmd_tokens;
static uint16_t ntp_tokens_per_packet;
static uint16_t cmd_tokens_per_packet;

/* Reduction of token rates to avoid overflow of 16-bit counters.  Negative
   shift is used for coarse limiting with intervals shorter than -TS_FRAC. */
static int ntp_token_shift;
static int cmd_token_shift;

/* Rates at which responses are randomly allowed (in log2) when the
   buckets don't have enough tokens.  This is necessary in order to
   prevent an attacker sending requests with spoofed source address
   from blocking responses to the address completely. */

#define MIN_LEAK_RATE 1
#define MAX_LEAK_RATE 4

static int ntp_leak_rate;
static int cmd_leak_rate;

/* Flag indicating whether the last response was dropped */
#define FLAG_NTP_DROPPED 0x1

/* NTP limit interval in log2 */
static int ntp_limit_interval;

/* Flag indicating whether facility is turned on or not */
static int active;

/* Global statistics */
static uint32_t total_ntp_hits;
static uint32_t total_cmd_hits;
static uint32_t total_ntp_drops;
static uint32_t total_cmd_drops;
static uint32_t total_record_drops;

#define NSEC_PER_SEC 1000000000U

/* ================================================== */

static int expand_hashtable(void);

/* ================================================== */

static int
compare_ts(uint32_t x, uint32_t y)
{
  if (x == y)
    return 0;
  if (y == INVALID_TS)
    return 1;
  return (int32_t)(x - y) > 0 ? 1 : -1;
}

/* ================================================== */

static Record *
get_record(IPAddr *ip)
{
  unsigned int first, i;
  time_t last_hit, oldest_hit = 0;
  Record *record, *oldest_record;

  if (!active || (ip->family != IPADDR_INET4 && ip->family != IPADDR_INET6))
    return NULL;

  while (1) {
    /* Get index of the first record in the slot */
    first = UTI_IPToHash(ip) % slots * SLOT_SIZE;

    for (i = 0, oldest_record = NULL; i < SLOT_SIZE; i++) {
      record = ARR_GetElement(records, first + i);

      if (!UTI_CompareIPs(ip, &record->ip_addr, NULL))
        return record;

      if (record->ip_addr.family == IPADDR_UNSPEC)
        break;

      last_hit = compare_ts(record->last_ntp_hit, record->last_cmd_hit) > 0 ?
                 record->last_ntp_hit : record->last_cmd_hit;

      if (!oldest_record || compare_ts(oldest_hit, last_hit) > 0 ||
          (oldest_hit == last_hit && record->ntp_hits + record->cmd_hits <
           oldest_record->ntp_hits + oldest_record->cmd_hits)) {
        oldest_record = record;
        oldest_hit = last_hit;
      }
    }

    /* If the slot still has an empty record, use it */
    if (record->ip_addr.family == IPADDR_UNSPEC)
      break;

    /* Resize the table if possible and try again as the new slot may
       have some empty records */
    if (expand_hashtable())
      continue;

    /* There is no other option, replace the oldest record */
    record = oldest_record;
    total_record_drops++;
    break;
  }

  record->ip_addr = *ip;
  record->last_ntp_hit = record->last_cmd_hit = INVALID_TS;
  record->ntp_hits = record->cmd_hits = 0;
  record->ntp_drops = record->cmd_drops = 0;
  record->ntp_tokens = max_ntp_tokens;
  record->cmd_tokens = max_cmd_tokens;
  record->ntp_rate = record->cmd_rate = INVALID_RATE;
  record->ntp_timeout_rate = INVALID_RATE;
  record->flags = 0;
  UTI_ZeroNtp64(&record->ntp_rx_ts);
  UTI_ZeroNtp64(&record->ntp_tx_ts);

  return record;
}

/* ================================================== */

static int
expand_hashtable(void)
{
  ARR_Instance old_records;
  Record *old_record, *new_record;
  unsigned int i;

  old_records = records;

  if (2 * slots > max_slots)
    return 0;

  records = ARR_CreateInstance(sizeof (Record));

  slots = MAX(MIN_SLOTS, 2 * slots);
  assert(slots <= max_slots);

  ARR_SetSize(records, slots * SLOT_SIZE);

  /* Mark all new records as empty */
  for (i = 0; i < slots * SLOT_SIZE; i++) {
    new_record = ARR_GetElement(records, i);
    new_record->ip_addr.family = IPADDR_UNSPEC;
  }

  if (!old_records)
    return 1;

  /* Copy old records to the new hash table */
  for (i = 0; i < ARR_GetSize(old_records); i++) {
    old_record = ARR_GetElement(old_records, i);
    if (old_record->ip_addr.family == IPADDR_UNSPEC)
      continue;

    new_record = get_record(&old_record->ip_addr);

    assert(new_record);
    *new_record = *old_record;
  }

  ARR_DestroyInstance(old_records);

  return 1;
}

/* ================================================== */

static void
set_bucket_params(int interval, int burst, uint16_t *max_tokens,
                  uint16_t *tokens_per_packet, int *token_shift)
{
  interval = CLAMP(MIN_LIMIT_INTERVAL, interval, MAX_LIMIT_INTERVAL);
  burst = CLAMP(MIN_LIMIT_BURST, burst, MAX_LIMIT_BURST);

  if (interval >= -TS_FRAC) {
    /* Find the smallest shift with which the maximum number fits in 16 bits */
    for (*token_shift = 0; *token_shift < interval + TS_FRAC; (*token_shift)++) {
      if (burst << (TS_FRAC + interval - *token_shift) < 1U << 16)
        break;
    }
  } else {
    /* Coarse rate limiting */
    *token_shift = interval + TS_FRAC;
    *tokens_per_packet = 1;
    burst = MAX(1U << -*token_shift, burst);
  }

  *tokens_per_packet = 1U << (TS_FRAC + interval - *token_shift);
  *max_tokens = *tokens_per_packet * burst;

  DEBUG_LOG("Tokens max %d packet %d shift %d",
            *max_tokens, *tokens_per_packet, *token_shift);
}

/* ================================================== */

void
CLG_Initialise(void)
{
  int interval, burst, leak_rate;

  max_ntp_tokens = max_cmd_tokens = 0;
  ntp_tokens_per_packet = cmd_tokens_per_packet = 0;
  ntp_token_shift = cmd_token_shift = 0;
  ntp_leak_rate = cmd_leak_rate = 0;
  ntp_limit_interval = MIN_LIMIT_INTERVAL;

  if (CNF_GetNTPRateLimit(&interval, &burst, &leak_rate)) {
    set_bucket_params(interval, burst, &max_ntp_tokens, &ntp_tokens_per_packet,
                      &ntp_token_shift);
    ntp_leak_rate = CLAMP(MIN_LEAK_RATE, leak_rate, MAX_LEAK_RATE);
    ntp_limit_interval = CLAMP(MIN_LIMIT_INTERVAL, interval, MAX_LIMIT_INTERVAL);
  }

  if (CNF_GetCommandRateLimit(&interval, &burst, &leak_rate)) {
    set_bucket_params(interval, burst, &max_cmd_tokens, &cmd_tokens_per_packet,
                      &cmd_token_shift);
    cmd_leak_rate = CLAMP(MIN_LEAK_RATE, leak_rate, MAX_LEAK_RATE);
  }

  active = !CNF_GetNoClientLog();
  if (!active) {
    if (ntp_leak_rate || cmd_leak_rate)
      LOG_FATAL("ratelimit cannot be used with noclientlog");
    return;
  }

  /* Calculate the maximum number of slots that can be allocated in the
     configured memory limit.  Take into account expanding of the hash
     table where two copies exist at the same time. */
  max_slots = CNF_GetClientLogLimit() / (sizeof (Record) * SLOT_SIZE * 3 / 2);
  max_slots = CLAMP(MIN_SLOTS, max_slots, MAX_SLOTS);

  slots = 0;
  records = NULL;

  expand_hashtable();

  UTI_GetRandomBytes(&ts_offset, sizeof (ts_offset));
  ts_offset %= NSEC_PER_SEC / (1U << TS_FRAC);
}

/* ================================================== */

void
CLG_Finalise(void)
{
  if (!active)
    return;

  ARR_DestroyInstance(records);
}

/* ================================================== */

static uint32_t
get_ts_from_timespec(struct timespec *ts)
{
  uint32_t sec = ts->tv_sec, nsec = ts->tv_nsec;

  nsec += ts_offset;
  if (nsec >= NSEC_PER_SEC) {
    nsec -= NSEC_PER_SEC;
    sec++;
  }

  /* This is fast and accurate enough */
  return sec << TS_FRAC | (140740U * (nsec >> 15)) >> (32 - TS_FRAC);
}

/* ================================================== */

static void
update_record(struct timespec *now, uint32_t *last_hit, uint32_t *hits,
              uint16_t *tokens, uint32_t max_tokens, int token_shift, int8_t *rate)
{
  uint32_t interval, now_ts, prev_hit, new_tokens;
  int interval2;

  now_ts = get_ts_from_timespec(now);

  prev_hit = *last_hit;
  *last_hit = now_ts;
  (*hits)++;

  interval = now_ts - prev_hit;

  if (prev_hit == INVALID_TS || (int32_t)interval < 0)
    return;

  if (token_shift >= 0)
    new_tokens = (now_ts >> token_shift) - (prev_hit >> token_shift);
  else if (now_ts - prev_hit > max_tokens)
    new_tokens = max_tokens;
  else
    new_tokens = (now_ts - prev_hit) << -token_shift;
  *tokens = MIN(*tokens + new_tokens, max_tokens);

  /* Convert the interval to scaled and rounded log2 */
  if (interval) {
    interval += interval >> 1;
    for (interval2 = -RATE_SCALE * TS_FRAC; interval2 < -MIN_RATE;
         interval2 += RATE_SCALE) {
      if (interval <= 1)
        break;
      interval >>= 1;
    }
  } else {
    interval2 = -RATE_SCALE * (TS_FRAC + 1);
  }

  /* Update the rate in a rough approximation of exponential moving average */
  if (*rate == INVALID_RATE) {
    *rate = -interval2;
  } else {
    if (*rate < -interval2) {
      (*rate)++;
    } else if (*rate > -interval2) {
      if (*rate > RATE_SCALE * 5 / 2 - interval2)
        *rate = RATE_SCALE * 5 / 2 - interval2;
      else
        *rate = (*rate - interval2 - 1) / 2;
    }
  }
}

/* ================================================== */

static int
get_index(Record *record)
{
  return record - (Record *)ARR_GetElements(records);
}

/* ================================================== */

int
CLG_GetClientIndex(IPAddr *client)
{
  Record *record;

  record = get_record(client);
  if (record == NULL)
    return -1;

  return get_index(record);
}

/* ================================================== */

int
CLG_LogNTPAccess(IPAddr *client, struct timespec *now)
{
  Record *record;

  total_ntp_hits++;

  record = get_record(client);
  if (record == NULL)
    return -1;

  /* Update one of the two rates depending on whether the previous request
     of the client had a reply or it timed out */
  update_record(now, &record->last_ntp_hit, &record->ntp_hits,
                &record->ntp_tokens, max_ntp_tokens, ntp_token_shift,
                record->flags & FLAG_NTP_DROPPED ?
                &record->ntp_timeout_rate : &record->ntp_rate);

  DEBUG_LOG("NTP hits %"PRIu32" rate %d trate %d tokens %d",
            record->ntp_hits, record->ntp_rate, record->ntp_timeout_rate,
            record->ntp_tokens);

  return get_index(record);
}

/* ================================================== */

int
CLG_LogCommandAccess(IPAddr *client, struct timespec *now)
{
  Record *record;

  total_cmd_hits++;

  record = get_record(client);
  if (record == NULL)
    return -1;

  update_record(now, &record->last_cmd_hit, &record->cmd_hits,
                &record->cmd_tokens, max_cmd_tokens, cmd_token_shift,
                &record->cmd_rate);

  DEBUG_LOG("Cmd hits %"PRIu32" rate %d tokens %d",
            record->cmd_hits, record->cmd_rate, record->cmd_tokens);

  return get_index(record);
}

/* ================================================== */

static int
limit_response_random(int leak_rate)
{
  static uint32_t rnd;
  static int bits_left = 0;
  int r;

  if (bits_left < leak_rate) {
    UTI_GetRandomBytes(&rnd, sizeof (rnd));
    bits_left = 8 * sizeof (rnd);
  }

  /* Return zero on average once per 2^leak_rate */
  r = rnd % (1U << leak_rate) ? 1 : 0;
  rnd >>= leak_rate;
  bits_left -= leak_rate;

  return r;
}

/* ================================================== */

int
CLG_LimitNTPResponseRate(int index)
{
  Record *record;
  int drop;

  if (!ntp_tokens_per_packet)
    return 0;

  record = ARR_GetElement(records, index);
  record->flags &= ~FLAG_NTP_DROPPED;

  if (record->ntp_tokens >= ntp_tokens_per_packet) {
    record->ntp_tokens -= ntp_tokens_per_packet;
    return 0;
  }

  drop = limit_response_random(ntp_leak_rate);

  /* Poorly implemented clients may send new requests at even a higher rate
     when they are not getting replies.  If the request rate seems to be more
     than twice as much as when replies are sent, give up on rate limiting to
     reduce the amount of traffic.  Invert the sense of the leak to respond to
     most of the requests, but still keep the estimated rate updated. */
  if (record->ntp_timeout_rate != INVALID_RATE &&
      record->ntp_timeout_rate > record->ntp_rate + RATE_SCALE)
    drop = !drop;

  if (!drop) {
    record->ntp_tokens = 0;
    return 0;
  }

  record->flags |= FLAG_NTP_DROPPED;
  record->ntp_drops++;
  total_ntp_drops++;

  return 1;
}

/* ================================================== */

int
CLG_LimitCommandResponseRate(int index)
{
  Record *record;

  if (!cmd_tokens_per_packet)
    return 0;

  record = ARR_GetElement(records, index);

  if (record->cmd_tokens >= cmd_tokens_per_packet) {
    record->cmd_tokens -= cmd_tokens_per_packet;
    return 0;
  }

  if (!limit_response_random(cmd_leak_rate)) {
    record->cmd_tokens = 0;
    return 0;
  }

  record->cmd_drops++;
  total_cmd_drops++;

  return 1;
}

/* ================================================== */

void CLG_GetNtpTimestamps(int index, NTP_int64 **rx_ts, NTP_int64 **tx_ts)
{
  Record *record;

  record = ARR_GetElement(records, index);

  *rx_ts = &record->ntp_rx_ts;
  *tx_ts = &record->ntp_tx_ts;
}

/* ================================================== */

int
CLG_GetNtpMinPoll(void)
{
  return ntp_limit_interval;
}

/* ================================================== */

int
CLG_GetNumberOfIndices(void)
{
  if (!active)
    return -1;

  return ARR_GetSize(records);
}

/* ================================================== */

static int get_interval(int rate)
{
  if (rate == INVALID_RATE)
    return 127;

  rate += rate > 0 ? RATE_SCALE / 2 : -RATE_SCALE / 2;

  return rate / -RATE_SCALE;
}

/* ================================================== */

static uint32_t get_last_ago(uint32_t x, uint32_t y)
{
  if (y == INVALID_TS || (int32_t)(x - y) < 0)
    return -1;

  return (x - y) >> TS_FRAC;
}

/* ================================================== */

int
CLG_GetClientAccessReportByIndex(int index, RPT_ClientAccessByIndex_Report *report, struct timespec *now)
{
  Record *record;
  uint32_t now_ts;

  if (!active || index < 0 || index >= ARR_GetSize(records))
    return 0;

  record = ARR_GetElement(records, index);

  if (record->ip_addr.family == IPADDR_UNSPEC)
    return 0;

  now_ts = get_ts_from_timespec(now);

  report->ip_addr = record->ip_addr;
  report->ntp_hits = record->ntp_hits;
  report->cmd_hits = record->cmd_hits;
  report->ntp_drops = record->ntp_drops;
  report->cmd_drops = record->cmd_drops;
  report->ntp_interval = get_interval(record->ntp_rate);
  report->cmd_interval = get_interval(record->cmd_rate);
  report->ntp_timeout_interval = get_interval(record->ntp_timeout_rate);
  report->last_ntp_hit_ago = get_last_ago(now_ts, record->last_ntp_hit);
  report->last_cmd_hit_ago = get_last_ago(now_ts, record->last_cmd_hit);

  return 1;
}

/* ================================================== */

void
CLG_GetServerStatsReport(RPT_ServerStatsReport *report)
{
  report->ntp_hits = total_ntp_hits;
  report->cmd_hits = total_cmd_hits;
  report->ntp_drops = total_ntp_drops;
  report->cmd_drops = total_cmd_drops;
  report->log_drops = total_record_drops;
}
