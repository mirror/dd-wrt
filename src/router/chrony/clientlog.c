/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * Copyright (C) Miroslav Lichvar  2009, 2015-2017, 2021, 2024
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
#include "local.h"
#include "memory.h"
#include "ntp.h"
#include "reports.h"
#include "util.h"
#include "logging.h"

#define MAX_SERVICES 3

typedef struct {
  IPAddr ip_addr;
  uint32_t last_hit[MAX_SERVICES];
  uint32_t hits[MAX_SERVICES];
  uint16_t drops[MAX_SERVICES];
  uint16_t tokens[MAX_SERVICES];
  int8_t rate[MAX_SERVICES];
  int8_t ntp_timeout_rate;
  uint8_t drop_flags;
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

static uint16_t max_tokens[MAX_SERVICES];
static uint16_t tokens_per_hit[MAX_SERVICES];

/* Reduction of token rates to avoid overflow of 16-bit counters.  Negative
   shift is used for coarse limiting with intervals shorter than -TS_FRAC. */
static int token_shift[MAX_SERVICES];

/* Rates at which responses are randomly allowed (in log2) when the
   buckets don't have enough tokens.  This is necessary in order to
   prevent an attacker sending requests with spoofed source address
   from blocking responses to the address completely. */

#define MIN_LEAK_RATE 1
#define MAX_LEAK_RATE 4

static int leak_rate[MAX_SERVICES];

/* Rates at which responses requesting clients to reduce their rate
   (e.g. NTP KoD RATE) are randomly allowed (in log2, but 0 means disabled) */

#define MIN_KOD_RATE 0
#define MAX_KOD_RATE 4

static int kod_rate[MAX_SERVICES];

/* Limit intervals in log2 */
static int limit_interval[MAX_SERVICES];

/* Flag indicating whether facility is turned on or not */
static int active;

/* RX and TX timestamp saved for clients using interleaved mode */
typedef struct {
  uint64_t rx_ts;
  uint8_t flags;
  uint8_t tx_ts_source;
  uint16_t slew_epoch;
  int32_t tx_ts_offset;
} NtpTimestamps;

/* Flags for NTP timestamps */
#define NTPTS_DISABLED 1
#define NTPTS_VALID_TX 2

/* RX->TX map using a circular buffer with ordered timestamps */
typedef struct {
  ARR_Instance timestamps;
  uint32_t first;
  uint32_t size;
  uint32_t max_size;
  uint32_t cached_index;
  uint64_t cached_rx_ts;
  uint16_t slew_epoch;
  double slew_offset;
} NtpTimestampMap;

static NtpTimestampMap ntp_ts_map;

/* Maximum interval of NTP timestamps in future after a backward step */
#define NTPTS_FUTURE_LIMIT (1LL << 32) /* 1 second */

/* Maximum number of timestamps moved in the array to insert a new timestamp */
#define NTPTS_INSERT_LIMIT 64

/* Maximum expected value of the timestamp source */
#define MAX_NTP_TS NTP_TS_HARDWARE

/* Global statistics */
static uint64_t total_hits[MAX_SERVICES];
static uint64_t total_drops[MAX_SERVICES];
static uint64_t total_ntp_auth_hits;
static uint64_t total_ntp_interleaved_hits;
static uint64_t total_record_drops;
static uint64_t total_ntp_rx_timestamps[MAX_NTP_TS + 1];
static uint64_t total_ntp_tx_timestamps[MAX_NTP_TS + 1];

#define NSEC_PER_SEC 1000000000U

/* ================================================== */

static int expand_hashtable(void);
static void handle_slew(struct timespec *raw, struct timespec *cooked, double dfreq,
                        double doffset, LCL_ChangeType change_type, void *anything);

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

static int
compare_total_hits(Record *x, Record *y)
{
  uint32_t x_hits, y_hits;
  int i;

  for (i = 0, x_hits = y_hits = 0; i < MAX_SERVICES; i++) {
    x_hits += x->hits[i];
    y_hits += y->hits[i];
  }

  return x_hits > y_hits ? 1 : -1;
}

/* ================================================== */

static Record *
get_record(IPAddr *ip)
{
  uint32_t last_hit = 0, oldest_hit = 0;
  Record *record, *oldest_record;
  unsigned int first, i, j;

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

      for (j = 0; j < MAX_SERVICES; j++) {
        if (j == 0 || compare_ts(last_hit, record->last_hit[j]) < 0)
          last_hit = record->last_hit[j];
      }

      if (!oldest_record || compare_ts(oldest_hit, last_hit) > 0 ||
          (oldest_hit == last_hit && compare_total_hits(oldest_record, record) > 0)) {
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
  for (i = 0; i < MAX_SERVICES; i++) {
    record->last_hit[i] = INVALID_TS;
    record->hits[i] = 0;
    record->drops[i] = 0;
    record->tokens[i] = max_tokens[i];
    record->rate[i] = INVALID_RATE;
  }
  record->ntp_timeout_rate = INVALID_RATE;
  record->drop_flags = 0;

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
  int i, interval, burst, lrate, krate, slots2;

  for (i = 0; i < MAX_SERVICES; i++) {
    max_tokens[i] = 0;
    tokens_per_hit[i] = 0;
    token_shift[i] = 0;
    leak_rate[i] = 0;
    kod_rate[i] = 0;
    limit_interval[i] = MIN_LIMIT_INTERVAL;

    switch (i) {
      case CLG_NTP:
        if (!CNF_GetNTPRateLimit(&interval, &burst, &lrate, &krate))
          continue;
        break;
      case CLG_NTSKE:
        if (!CNF_GetNtsRateLimit(&interval, &burst, &lrate))
          continue;
        break;
      case CLG_CMDMON:
        if (!CNF_GetCommandRateLimit(&interval, &burst, &lrate))
          continue;
        break;
      default:
        assert(0);
    }

    set_bucket_params(interval, burst, &max_tokens[i], &tokens_per_hit[i], &token_shift[i]);
    leak_rate[i] = CLAMP(MIN_LEAK_RATE, lrate, MAX_LEAK_RATE);
    kod_rate[i] = CLAMP(MIN_KOD_RATE, krate, MAX_KOD_RATE);
    limit_interval[i] = CLAMP(MIN_LIMIT_INTERVAL, interval, MAX_LIMIT_INTERVAL);
  }

  active = !CNF_GetNoClientLog();
  if (!active) {
    for (i = 0; i < MAX_SERVICES; i++) {
      if (leak_rate[i] != 0)
        LOG_FATAL("Rate limiting cannot be enabled with noclientlog");
    }
    return;
  }

  /* Calculate the maximum number of slots that can be allocated in the
     configured memory limit.  Take into account expanding of the hash
     table where two copies exist at the same time. */
  max_slots = CNF_GetClientLogLimit() /
              ((sizeof (Record) + sizeof (NtpTimestamps)) * SLOT_SIZE * 3 / 2);
  max_slots = CLAMP(MIN_SLOTS, max_slots, MAX_SLOTS);
  for (slots2 = 0; 1U << (slots2 + 1) <= max_slots; slots2++)
    ;

  DEBUG_LOG("Max records %u", 1U << (slots2 + SLOT_BITS));

  slots = 0;
  records = NULL;

  expand_hashtable();

  UTI_GetRandomBytes(&ts_offset, sizeof (ts_offset));
  ts_offset %= NSEC_PER_SEC / (1U << TS_FRAC);

  ntp_ts_map.timestamps = NULL;
  ntp_ts_map.first = 0;
  ntp_ts_map.size = 0;
  ntp_ts_map.max_size = 1U << (slots2 + SLOT_BITS);
  ntp_ts_map.cached_index = 0;
  ntp_ts_map.cached_rx_ts = 0ULL;
  ntp_ts_map.slew_epoch = 0;
  ntp_ts_map.slew_offset = 0.0;

  LCL_AddParameterChangeHandler(handle_slew, NULL);
}

/* ================================================== */

void
CLG_Finalise(void)
{
  if (!active)
    return;

  ARR_DestroyInstance(records);
  if (ntp_ts_map.timestamps)
    ARR_DestroyInstance(ntp_ts_map.timestamps);

  LCL_RemoveParameterChangeHandler(handle_slew, NULL);
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
update_record(CLG_Service service, Record *record, struct timespec *now)
{
  uint32_t interval, now_ts, prev_hit, tokens;
  int interval2, tshift, mtokens;
  int8_t *rate;

  now_ts = get_ts_from_timespec(now);

  prev_hit = record->last_hit[service];
  record->last_hit[service] = now_ts;
  record->hits[service]++;

  interval = now_ts - prev_hit;

  if (prev_hit == INVALID_TS || (int32_t)interval < 0)
    return;

  tshift = token_shift[service];
  mtokens = max_tokens[service];

  if (tshift >= 0)
    tokens = (now_ts >> tshift) - (prev_hit >> tshift);
  else if (now_ts - prev_hit > mtokens)
    tokens = mtokens;
  else
    tokens = (now_ts - prev_hit) << -tshift;
  record->tokens[service] = MIN(record->tokens[service] + tokens, mtokens);

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

  /* For the NTP service, update one of the two rates depending on whether
     the previous request of the client had a reply or it timed out */
  rate = service == CLG_NTP && record->drop_flags & (1U << service) ?
           &record->ntp_timeout_rate : &record->rate[service];

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

static void
check_service_number(CLG_Service service)
{
  assert(service >= 0 && service <= MAX_SERVICES);
}

/* ================================================== */

int
CLG_LogServiceAccess(CLG_Service service, IPAddr *client, struct timespec *now)
{
  Record *record;

  check_service_number(service);

  total_hits[service]++;

  record = get_record(client);
  if (record == NULL)
    return -1;

  update_record(service, record, now);

  DEBUG_LOG("service %d hits %"PRIu32" rate %d trate %d tokens %d",
            (int)service, record->hits[service], record->rate[service],
            service == CLG_NTP ? record->ntp_timeout_rate : INVALID_RATE,
            record->tokens[service]);

  return get_index(record);
}

/* ================================================== */

static int
limit_response_random(int rate)
{
  static uint32_t rnd;
  static int bits_left = 0;
  int r;

  if (bits_left < rate) {
    UTI_GetRandomBytes(&rnd, sizeof (rnd));
    bits_left = 8 * sizeof (rnd);
  }

  /* Return zero on average once per 2^rate */
  r = rnd % (1U << rate) ? 1 : 0;
  rnd >>= rate;
  bits_left -= rate;

  return r;
}

/* ================================================== */

CLG_Limit
CLG_LimitServiceRate(CLG_Service service, int index)
{
  Record *record;
  int drop;

  check_service_number(service);

  if (tokens_per_hit[service] == 0)
    return CLG_PASS;

  record = ARR_GetElement(records, index);
  record->drop_flags &= ~(1U << service);

  if (record->tokens[service] >= tokens_per_hit[service]) {
    record->tokens[service] -= tokens_per_hit[service];
    return CLG_PASS;
  }

  drop = limit_response_random(leak_rate[service]);

  /* Poorly implemented NTP clients can send requests at a higher rate
     when they are not getting replies.  If the request rate seems to be more
     than twice as much as when replies are sent, give up on rate limiting to
     reduce the amount of traffic.  Invert the sense of the leak to respond to
     most of the requests, but still keep the estimated rate updated. */
  if (service == CLG_NTP && record->ntp_timeout_rate != INVALID_RATE &&
      record->ntp_timeout_rate > record->rate[service] + RATE_SCALE)
    drop = !drop;

  if (!drop) {
    record->tokens[service] = 0;
    return CLG_PASS;
  }

  if (kod_rate[service] > 0 && !limit_response_random(kod_rate[service])) {
    return CLG_KOD;
  }

  record->drop_flags |= 1U << service;
  record->drops[service]++;
  total_drops[service]++;

  return CLG_DROP;
}

/* ================================================== */

void
CLG_UpdateNtpStats(int auth, NTP_Timestamp_Source rx_ts_src, NTP_Timestamp_Source tx_ts_src)
{
  if (auth)
    total_ntp_auth_hits++;
  if (rx_ts_src >= 0 && rx_ts_src <= MAX_NTP_TS)
    total_ntp_rx_timestamps[rx_ts_src]++;
  if (tx_ts_src >= 0 && tx_ts_src <= MAX_NTP_TS)
    total_ntp_tx_timestamps[tx_ts_src]++;
}

/* ================================================== */

int
CLG_GetNtpMinPoll(void)
{
  return limit_interval[CLG_NTP];
}

/* ================================================== */

static NtpTimestamps *
get_ntp_tss(uint32_t index)
{
  return ARR_GetElement(ntp_ts_map.timestamps,
                        (ntp_ts_map.first + index) & (ntp_ts_map.max_size - 1));
}

/* ================================================== */

static int
find_ntp_rx_ts(uint64_t rx_ts, uint32_t *index)
{
  uint64_t rx_x, rx_lo, rx_hi, step;
  uint32_t i, x, lo, hi;

  if (ntp_ts_map.cached_rx_ts == rx_ts && rx_ts != 0ULL) {
    *index = ntp_ts_map.cached_index;
    return 1;
  }

  if (ntp_ts_map.size == 0) {
    *index = 0;
    return 0;
  }

  lo = 0;
  hi = ntp_ts_map.size - 1;
  rx_lo = get_ntp_tss(lo)->rx_ts;
  rx_hi = get_ntp_tss(hi)->rx_ts;

  /* Check for ts < lo before ts > hi to trim timestamps from "future" later
     if both conditions are true to not break the order of the endpoints.
     Compare timestamps by their difference to allow adjacent NTP eras. */
  if ((int64_t)(rx_ts - rx_lo) < 0) {
    *index = 0;
    return 0;
  } else if ((int64_t)(rx_ts - rx_hi) > 0) {
    *index = ntp_ts_map.size;
    return 0;
  }

  /* Perform a combined linear interpolation and binary search */

  for (i = 0; ; i++) {
    if (rx_ts == rx_hi) {
      *index = ntp_ts_map.cached_index = hi;
      ntp_ts_map.cached_rx_ts = rx_ts;
      return 1;
    } else if (rx_ts == rx_lo) {
      *index = ntp_ts_map.cached_index = lo;
      ntp_ts_map.cached_rx_ts = rx_ts;
      return 1;
    } else if (lo + 1 == hi) {
      *index = hi;
      return 0;
    }

    if (hi - lo > 3 && i % 2 == 0) {
      step = (rx_hi - rx_lo) / (hi - lo);
      if (step == 0)
        step = 1;
      x = lo + (rx_ts - rx_lo) / step;
    } else {
      x = lo + (hi - lo) / 2;
    }

    if (x <= lo)
      x = lo + 1;
    else if (x >= hi)
      x = hi - 1;

    rx_x = get_ntp_tss(x)->rx_ts;

    if ((int64_t)(rx_x - rx_ts) <= 0) {
      lo = x;
      rx_lo = rx_x;
    } else {
      hi = x;
      rx_hi = rx_x;
    }
  }
}

/* ================================================== */

static uint64_t
ntp64_to_int64(NTP_int64 *ts)
{
  return (uint64_t)ntohl(ts->hi) << 32 | ntohl(ts->lo);
}

/* ================================================== */

static void
int64_to_ntp64(uint64_t ts, NTP_int64 *ntp_ts)
{
  ntp_ts->hi = htonl(ts >> 32);
  ntp_ts->lo = htonl(ts);
}

/* ================================================== */

static uint32_t
push_ntp_tss(uint32_t index)
{
  if (ntp_ts_map.size < ntp_ts_map.max_size) {
    ntp_ts_map.size++;
  } else {
    ntp_ts_map.first = (ntp_ts_map.first + 1) % (ntp_ts_map.max_size);
    if (index > 0)
      index--;
  }

  return index;
}

/* ================================================== */

static void
set_ntp_tx(NtpTimestamps *tss, NTP_int64 *rx_ts, struct timespec *tx_ts,
           NTP_Timestamp_Source tx_src)
{
  struct timespec ts;

  if (!tx_ts) {
    tss->flags &= ~NTPTS_VALID_TX;
    return;
  }

  UTI_Ntp64ToTimespec(rx_ts, &ts);
  UTI_DiffTimespecs(&ts, tx_ts, &ts);

  if (ts.tv_sec < -2 || ts.tv_sec > 1) {
    tss->flags &= ~NTPTS_VALID_TX;
    return;
  }

  tss->tx_ts_offset = (int32_t)ts.tv_nsec + (int32_t)ts.tv_sec * (int32_t)NSEC_PER_SEC;
  tss->flags |= NTPTS_VALID_TX;
  tss->tx_ts_source = tx_src;
}

/* ================================================== */

static void
get_ntp_tx(NtpTimestamps *tss, struct timespec *tx_ts, NTP_Timestamp_Source *tx_src)
{
  int32_t offset = tss->tx_ts_offset;
  NTP_int64 ntp_ts;

  if (tss->flags & NTPTS_VALID_TX) {
    int64_to_ntp64(tss->rx_ts, &ntp_ts);
    UTI_Ntp64ToTimespec(&ntp_ts, tx_ts);
    if (offset >= (int32_t)NSEC_PER_SEC) {
      offset -= NSEC_PER_SEC;
      tx_ts->tv_sec++;
    }
    tx_ts->tv_nsec += offset;
    UTI_NormaliseTimespec(tx_ts);
  } else {
    UTI_ZeroTimespec(tx_ts);
  }

  *tx_src = tss->tx_ts_source;
}

/* ================================================== */

void
CLG_SaveNtpTimestamps(NTP_int64 *rx_ts, struct timespec *tx_ts, NTP_Timestamp_Source tx_src)
{
  NtpTimestamps *tss;
  uint32_t i, index;
  uint64_t rx;

  if (!active)
    return;

  /* Allocate the array on first use */
  if (!ntp_ts_map.timestamps) {
    ntp_ts_map.timestamps = ARR_CreateInstance(sizeof (NtpTimestamps));
    ARR_SetSize(ntp_ts_map.timestamps, ntp_ts_map.max_size);
  }

  rx = ntp64_to_int64(rx_ts);

  if (rx == 0ULL)
    return;

  /* Disable the RX timestamp if it already exists to avoid responding
     with a wrong TX timestamp */
  if (find_ntp_rx_ts(rx, &index)) {
    get_ntp_tss(index)->flags |= NTPTS_DISABLED;
    return;
  }

  assert(index <= ntp_ts_map.size);

  if (index == ntp_ts_map.size) {
    /* Increase the size or drop the oldest timestamp to make room for
       the new timestamp */
    index = push_ntp_tss(index);
  } else {
    /* Trim timestamps in distant future after backward step */
    while (index < ntp_ts_map.size &&
           get_ntp_tss(ntp_ts_map.size - 1)->rx_ts - rx > NTPTS_FUTURE_LIMIT)
      ntp_ts_map.size--;

    /* Insert the timestamp if it is close to the latest timestamp.
       Otherwise, replace the closest older or the oldest timestamp. */
    if (index + NTPTS_INSERT_LIMIT >= ntp_ts_map.size) {
      index = push_ntp_tss(index);
      for (i = ntp_ts_map.size - 1; i > index; i--)
        *get_ntp_tss(i) = *get_ntp_tss(i - 1);
    } else {
      if (index > 0)
        index--;
    }
  }

  ntp_ts_map.cached_index = index;
  ntp_ts_map.cached_rx_ts = rx;

  tss = get_ntp_tss(index);
  tss->rx_ts = rx;
  tss->flags = 0;
  tss->slew_epoch = ntp_ts_map.slew_epoch;
  set_ntp_tx(tss, rx_ts, tx_ts, tx_src);

  DEBUG_LOG("Saved RX+TX index=%"PRIu32" first=%"PRIu32" size=%"PRIu32,
            index, ntp_ts_map.first, ntp_ts_map.size);
}

/* ================================================== */

static void
handle_slew(struct timespec *raw, struct timespec *cooked, double dfreq,
            double doffset, LCL_ChangeType change_type, void *anything)
{
  /* Drop all timestamps on unknown step */
  if (change_type == LCL_ChangeUnknownStep) {
    ntp_ts_map.size = 0;
    ntp_ts_map.cached_rx_ts = 0ULL;
  }

  ntp_ts_map.slew_epoch++;
  ntp_ts_map.slew_offset = doffset;
}

/* ================================================== */

void
CLG_UndoNtpTxTimestampSlew(NTP_int64 *rx_ts, struct timespec *tx_ts)
{
  uint32_t index;

  if (!ntp_ts_map.timestamps)
    return;

  if (!find_ntp_rx_ts(ntp64_to_int64(rx_ts), &index))
    return;

  /* If the RX timestamp was captured before the last correction of the clock,
     remove the adjustment from the TX timestamp */
  if ((uint16_t)(get_ntp_tss(index)->slew_epoch + 1U) == ntp_ts_map.slew_epoch)
    UTI_AddDoubleToTimespec(tx_ts, ntp_ts_map.slew_offset, tx_ts);
}

/* ================================================== */

void
CLG_UpdateNtpTxTimestamp(NTP_int64 *rx_ts, struct timespec *tx_ts,
                         NTP_Timestamp_Source tx_src)
{
  uint32_t index;

  if (!ntp_ts_map.timestamps)
    return;

  if (!find_ntp_rx_ts(ntp64_to_int64(rx_ts), &index))
    return;

  set_ntp_tx(get_ntp_tss(index), rx_ts, tx_ts, tx_src);
}

/* ================================================== */

int
CLG_GetNtpTxTimestamp(NTP_int64 *rx_ts, struct timespec *tx_ts,
                      NTP_Timestamp_Source *tx_src)
{
  NtpTimestamps *tss;
  uint32_t index;

  if (!ntp_ts_map.timestamps)
    return 0;

  if (!find_ntp_rx_ts(ntp64_to_int64(rx_ts), &index))
    return 0;

  tss = get_ntp_tss(index);

  if (tss->flags & NTPTS_DISABLED)
    return 0;

  get_ntp_tx(tss, tx_ts, tx_src);

  return 1;
}

/* ================================================== */

void
CLG_DisableNtpTimestamps(NTP_int64 *rx_ts)
{
  uint32_t index;

  if (!ntp_ts_map.timestamps)
    return;

  if (find_ntp_rx_ts(ntp64_to_int64(rx_ts), &index))
    get_ntp_tss(index)->flags |= NTPTS_DISABLED;

  /* This assumes the function is called only to prevent multiple
     interleaved responses to the same timestamp */
  total_ntp_interleaved_hits++;
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
CLG_GetClientAccessReportByIndex(int index, int reset, uint32_t min_hits,
                                 RPT_ClientAccessByIndex_Report *report, struct timespec *now)
{
  Record *record;
  uint32_t now_ts;
  int i, r;

  if (!active || index < 0 || index >= ARR_GetSize(records))
    return 0;

  record = ARR_GetElement(records, index);

  if (record->ip_addr.family == IPADDR_UNSPEC)
    return 0;

  if (min_hits == 0) {
    r = 1;
  } else {
    for (i = r = 0; i < MAX_SERVICES; i++) {
      if (record->hits[i] >= min_hits) {
        r = 1;
        break;
      }
    }
  }

  if (r) {
    now_ts = get_ts_from_timespec(now);

    report->ip_addr = record->ip_addr;
    report->ntp_hits = record->hits[CLG_NTP];
    report->nke_hits = record->hits[CLG_NTSKE];
    report->cmd_hits = record->hits[CLG_CMDMON];
    report->ntp_drops = record->drops[CLG_NTP];
    report->nke_drops = record->drops[CLG_NTSKE];
    report->cmd_drops = record->drops[CLG_CMDMON];
    report->ntp_interval = get_interval(record->rate[CLG_NTP]);
    report->nke_interval = get_interval(record->rate[CLG_NTSKE]);
    report->cmd_interval = get_interval(record->rate[CLG_CMDMON]);
    report->ntp_timeout_interval = get_interval(record->ntp_timeout_rate);
    report->last_ntp_hit_ago = get_last_ago(now_ts, record->last_hit[CLG_NTP]);
    report->last_nke_hit_ago = get_last_ago(now_ts, record->last_hit[CLG_NTSKE]);
    report->last_cmd_hit_ago = get_last_ago(now_ts, record->last_hit[CLG_CMDMON]);
  }

  if (reset) {
    for (i = 0; i < MAX_SERVICES; i++) {
      record->hits[i] = 0;
      record->drops[i] = 0;
    }
  }

  return r;
}

/* ================================================== */

void
CLG_GetServerStatsReport(RPT_ServerStatsReport *report)
{
  report->ntp_hits = total_hits[CLG_NTP];
  report->nke_hits = total_hits[CLG_NTSKE];
  report->cmd_hits = total_hits[CLG_CMDMON];
  report->ntp_drops = total_drops[CLG_NTP];
  report->nke_drops = total_drops[CLG_NTSKE];
  report->cmd_drops = total_drops[CLG_CMDMON];
  report->log_drops = total_record_drops;
  report->ntp_auth_hits = total_ntp_auth_hits;
  report->ntp_interleaved_hits = total_ntp_interleaved_hits;
  report->ntp_timestamps = ntp_ts_map.size;
  report->ntp_span_seconds = ntp_ts_map.size > 1 ?
                             (get_ntp_tss(ntp_ts_map.size - 1)->rx_ts -
                              get_ntp_tss(0)->rx_ts) >> 32 : 0;
  report->ntp_daemon_rx_timestamps = total_ntp_rx_timestamps[NTP_TS_DAEMON];
  report->ntp_daemon_tx_timestamps = total_ntp_tx_timestamps[NTP_TS_DAEMON];
  report->ntp_kernel_rx_timestamps = total_ntp_rx_timestamps[NTP_TS_KERNEL];
  report->ntp_kernel_tx_timestamps = total_ntp_tx_timestamps[NTP_TS_KERNEL];
  report->ntp_hw_rx_timestamps = total_ntp_rx_timestamps[NTP_TS_HARDWARE];
  report->ntp_hw_tx_timestamps = total_ntp_tx_timestamps[NTP_TS_HARDWARE];
}
