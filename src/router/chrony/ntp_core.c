/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * Copyright (C) Miroslav Lichvar  2009-2018
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

  Core NTP protocol engine
  */

#include "config.h"

#include "sysincl.h"

#include "array.h"
#include "ntp_core.h"
#include "ntp_io.h"
#include "ntp_signd.h"
#include "memory.h"
#include "sched.h"
#include "reference.h"
#include "local.h"
#include "samplefilt.h"
#include "smooth.h"
#include "sources.h"
#include "util.h"
#include "conf.h"
#include "logging.h"
#include "keys.h"
#include "addrfilt.h"
#include "clientlog.h"

/* ================================================== */

static LOG_FileID logfileid;
static int log_raw_measurements;

/* ================================================== */
/* Enumeration used for remembering the operating mode of one of the
   sources */

typedef enum {
  MD_OFFLINE,                   /* No sampling at all */
  MD_ONLINE,                    /* Normal sampling based on sampling interval */
  MD_BURST_WAS_OFFLINE,         /* Burst sampling, return to offline afterwards */
  MD_BURST_WAS_ONLINE,          /* Burst sampling, return to online afterwards */
} OperatingMode;

/* ================================================== */
/* Enumeration for authentication modes of NTP packets */

typedef enum {
  AUTH_NONE = 0,                /* No authentication */
  AUTH_SYMMETRIC,               /* MAC using symmetric key (RFC 1305, RFC 5905) */
  AUTH_MSSNTP,                  /* MS-SNTP authenticator field */
  AUTH_MSSNTP_EXT,              /* MS-SNTP extended authenticator field */
} AuthenticationMode;

/* ================================================== */
/* Structure used for holding a single peer/server's
   protocol machine */

struct NCR_Instance_Record {
  NTP_Remote_Address remote_addr; /* Needed for routing transmit packets */
  NTP_Local_Address local_addr; /* Local address/socket used to send packets */
  NTP_Mode mode;                /* The source's NTP mode
                                   (client/server or symmetric active peer) */
  int interleaved;              /* Boolean enabling interleaved NTP mode */
  OperatingMode opmode;         /* Whether we are sampling this source
                                   or not and in what way */
  SCH_TimeoutID rx_timeout_id;  /* Timeout ID for latest received response */
  SCH_TimeoutID tx_timeout_id;  /* Timeout ID for next transmission */
  int tx_suspended;             /* Boolean indicating we can't transmit yet */

  int auto_burst;               /* If 1, initiate a burst on each poll */
  int auto_offline;             /* If 1, automatically go offline when requests
                                   cannot be sent */

  int local_poll;               /* Log2 of polling interval at our end */
  int remote_poll;              /* Log2 of server/peer's polling interval (recovered
                                   from received packets) */
  int remote_stratum;           /* Stratum of the server/peer (recovered from
                                   received packets) */

  int presend_minpoll;           /* If the current polling interval is
                                    at least this, an extra client packet
                                    will be send some time before normal
                                    transmit.  This ensures that both
                                    us and the server/peer have an ARP
                                    entry for each other ready, which
                                    means our measurement is not
                                    botched by an ARP round-trip on one
                                    side or the other. */

  int presend_done;             /* The presend packet has been sent */

  int minpoll;                  /* Log2 of minimum defined polling interval */
  int maxpoll;                  /* Log2 of maximum defined polling interval */

  int min_stratum;              /* Increase stratum in received packets to the
                                   minimum */

  int poll_target;              /* Target number of sourcestats samples */

  int version;                  /* Version set in packets for server/peer */

  double poll_score;            /* Score of current local poll */

  double max_delay;             /* Maximum round-trip delay to the
                                   peer that we can tolerate and still
                                   use the sample for generating
                                   statistics from */

  double max_delay_ratio;       /* Largest ratio of delay /
                                   min_delay_in_register that we can
                                   tolerate.  */

  double max_delay_dev_ratio;   /* Maximum ratio of increase in delay / stddev */

  double offset_correction;     /* Correction applied to measured offset
                                   (e.g. for asymmetry in network delay) */

  AuthenticationMode auth_mode; /* Authentication mode of our requests */
  uint32_t auth_key_id;          /* The ID of the authentication key to
                                   use. */

  /* Count of transmitted packets since last valid response */
  unsigned int tx_count;

  /* Flag indicating a valid response was received since last request */
  int valid_rx;

  /* Flag indicating the timestamps below are from a valid packet and may
     be used for synchronisation */
  int valid_timestamps;

  /* Receive and transmit timestamps from the last valid response */
  NTP_int64 remote_ntp_rx;
  NTP_int64 remote_ntp_tx;

  /* Local timestamp when the last valid response was received from the
     source.  We have to be prepared to tinker with this if the local
     clock has its frequency adjusted before we repond.  The value we
     store here is what our own local time was when the same arrived.
     Before replying, we have to correct this to fit with the
     parameters for the current reference.  (It must be stored
     relative to local time to permit frequency and offset adjustments
     to be made when we trim the local clock). */
  NTP_int64 local_ntp_rx;
  NTP_Local_Timestamp local_rx;

  /* Local timestamp when we last transmitted a packet to the source.
     We store two versions.  The first is in NTP format, and is used
     to validate the next received packet from the source.
     Additionally, this is corrected to bring it into line with the
     current reference.  The second is in timespec format, and is kept
     relative to the local clock.  We modify this in accordance with
     local clock frequency/offset changes, and use this for computing
     statistics about the source when a return packet arrives. */
  NTP_int64 local_ntp_tx;
  NTP_Local_Timestamp local_tx;

  /* Previous values of some variables needed in interleaved mode */
  NTP_Local_Timestamp prev_local_tx;
  int prev_local_poll;
  unsigned int prev_tx_count;

  /* Flag indicating the two timestamps below were updated since the
     last transmission */
  int updated_init_timestamps;

  /* Timestamps used for (re)starting the symmetric protocol, when we
     need to respond to a packet which is not a valid response */
  NTP_int64 init_remote_ntp_tx;
  NTP_Local_Timestamp init_local_rx;

  /* The instance record in the main source management module.  This
     performs the statistical analysis on the samples we generate */

  SRC_Instance source;

  /* Optional median filter for NTP measurements */
  SPF_Instance filter;

  int burst_good_samples_to_go;
  int burst_total_samples_to_go;

  /* Report from last valid response */
  RPT_NTPReport report;
};

typedef struct {
  NTP_Remote_Address addr;
  NTP_Local_Address local_addr;
  int interval;
} BroadcastDestination;

/* Array of BroadcastDestination */
static ARR_Instance broadcasts;

/* ================================================== */
/* Initial delay period before first packet is transmitted (in seconds) */
#define INITIAL_DELAY 0.2

/* Spacing required between samples for any two servers/peers (to
   minimise risk of network collisions) (in seconds) */
#define MIN_SAMPLING_SEPARATION 0.002
#define MAX_SAMPLING_SEPARATION 0.2

/* Randomness added to spacing between samples for one server/peer */
#define SAMPLING_RANDOMNESS 0.02

/* Adjustment of the peer polling interval */
#define PEER_SAMPLING_ADJ 1.1

/* Maximum spacing between samples in the burst mode as an absolute
   value and ratio to the normal polling interval */
#define MAX_BURST_INTERVAL 2.0
#define MAX_BURST_POLL_RATIO 0.25

/* Number of samples in initial burst */
#define IBURST_GOOD_SAMPLES 4
#define IBURST_TOTAL_SAMPLES SOURCE_REACH_BITS

/* Number of samples in automatic burst */
#define BURST_GOOD_SAMPLES 1
#define MAX_BURST_TOTAL_SAMPLES 4

/* Time to wait after sending packet to 'warm up' link */
#define WARM_UP_DELAY 2.0

/* Compatible NTP protocol versions */
#define NTP_MAX_COMPAT_VERSION NTP_VERSION
#define NTP_MIN_COMPAT_VERSION 1

/* Maximum allowed dispersion - as defined in RFC 5905 (16 seconds) */
#define NTP_MAX_DISPERSION 16.0

/* Invalid stratum number */
#define NTP_INVALID_STRATUM 0

/* Maximum allowed time for server to process client packet */
#define MAX_SERVER_INTERVAL 4.0

/* Maximum acceptable delay in transmission for timestamp correction */
#define MAX_TX_DELAY 1.0

/* Maximum allowed values of maxdelay parameters */
#define MAX_MAXDELAY 1.0e3
#define MAX_MAXDELAYRATIO 1.0e6
#define MAX_MAXDELAYDEVRATIO 1.0e6

/* Minimum and maximum allowed poll interval */
#define MIN_POLL -6
#define MAX_POLL 24

/* Enable sub-second polling intervals only when the peer delay is not
   longer than 10 milliseconds to restrict them to local networks */
#define MIN_NONLAN_POLL 0
#define MAX_LAN_PEER_DELAY 0.01

/* Kiss-o'-Death codes */
#define KOD_RATE 0x52415445UL /* RATE */

/* Maximum poll interval set by KoD RATE */
#define MAX_KOD_RATE_POLL SRC_DEFAULT_MAXPOLL

/* Maximum number of missed responses to accept samples using old timestamps
   in the interleaved client/server mode */
#define MAX_CLIENT_INTERLEAVED_TX 4

/* Maximum ratio of local intervals in the timestamp selection of the
   interleaved mode to prefer a sample using previous timestamps */
#define MAX_INTERLEAVED_L2L_RATIO 0.1

/* Invalid socket, different from the one in ntp_io.c */
#define INVALID_SOCK_FD -2

/* ================================================== */

/* Server IPv4/IPv6 sockets */
static int server_sock_fd4;
static int server_sock_fd6;

static ADF_AuthTable access_auth_table;

/* Characters for printing synchronisation status and timestamping source */
static const char leap_chars[4] = {'N', '+', '-', '?'};
static const char tss_chars[3] = {'D', 'K', 'H'};

/* ================================================== */
/* Forward prototypes */

static void transmit_timeout(void *arg);
static double get_transmit_delay(NCR_Instance inst, int on_tx, double last_tx);
static double get_separation(int poll);

/* ================================================== */

static void
do_size_checks(void)
{
  /* Assertions to check the sizes of certain data types
     and the positions of certain record fields */

  /* Check that certain invariants are true */
  assert(sizeof(NTP_int32) == 4);
  assert(sizeof(NTP_int64) == 8);

  /* Check offsets of all fields in the NTP packet format */
  assert(offsetof(NTP_Packet, lvm)             ==  0);
  assert(offsetof(NTP_Packet, stratum)         ==  1);
  assert(offsetof(NTP_Packet, poll)            ==  2);
  assert(offsetof(NTP_Packet, precision)       ==  3);
  assert(offsetof(NTP_Packet, root_delay)      ==  4);
  assert(offsetof(NTP_Packet, root_dispersion) ==  8);
  assert(offsetof(NTP_Packet, reference_id)    == 12);
  assert(offsetof(NTP_Packet, reference_ts)    == 16);
  assert(offsetof(NTP_Packet, originate_ts)    == 24);
  assert(offsetof(NTP_Packet, receive_ts)      == 32);
  assert(offsetof(NTP_Packet, transmit_ts)     == 40);
}

/* ================================================== */

static void
do_time_checks(void)
{
  struct timespec now;
  time_t warning_advance = 3600 * 24 * 365 * 10; /* 10 years */

#ifdef HAVE_LONG_TIME_T
  /* Check that time before NTP_ERA_SPLIT underflows correctly */

  struct timespec ts1 = {NTP_ERA_SPLIT, 1}, ts2 = {NTP_ERA_SPLIT - 1, 1};
  NTP_int64 nts1, nts2;
  int r;

  UTI_TimespecToNtp64(&ts1, &nts1, NULL);
  UTI_TimespecToNtp64(&ts2, &nts2, NULL);
  UTI_Ntp64ToTimespec(&nts1, &ts1);
  UTI_Ntp64ToTimespec(&nts2, &ts2);

  r = ts1.tv_sec == NTP_ERA_SPLIT &&
      ts1.tv_sec + (1ULL << 32) - 1 == ts2.tv_sec;

  assert(r);

  LCL_ReadRawTime(&now);
  if (ts2.tv_sec - now.tv_sec < warning_advance)
    LOG(LOGS_WARN, "Assumed NTP time ends at %s!", UTI_TimeToLogForm(ts2.tv_sec));
#else
  LCL_ReadRawTime(&now);
  if (now.tv_sec > 0x7fffffff - warning_advance)
    LOG(LOGS_WARN, "System time ends at %s!", UTI_TimeToLogForm(0x7fffffff));
#endif
}

/* ================================================== */

static void
zero_local_timestamp(NTP_Local_Timestamp *ts)
{
  UTI_ZeroTimespec(&ts->ts);
  ts->err = 0.0;
  ts->source = NTP_TS_DAEMON;
}

/* ================================================== */

void
NCR_Initialise(void)
{
  do_size_checks();
  do_time_checks();

  logfileid = CNF_GetLogMeasurements(&log_raw_measurements) ? LOG_FileOpen("measurements",
      "   Date (UTC) Time     IP Address   L St 123 567 ABCD  LP RP Score    Offset  Peer del. Peer disp.  Root del. Root disp. Refid     MTxRx")
    : -1;

  access_auth_table = ADF_CreateTable();
  broadcasts = ARR_CreateInstance(sizeof (BroadcastDestination));

  /* Server socket will be opened when access is allowed */
  server_sock_fd4 = INVALID_SOCK_FD;
  server_sock_fd6 = INVALID_SOCK_FD;
}

/* ================================================== */

void
NCR_Finalise(void)
{
  unsigned int i;

  if (server_sock_fd4 != INVALID_SOCK_FD)
    NIO_CloseServerSocket(server_sock_fd4);
  if (server_sock_fd6 != INVALID_SOCK_FD)
    NIO_CloseServerSocket(server_sock_fd6);

  for (i = 0; i < ARR_GetSize(broadcasts); i++)
    NIO_CloseServerSocket(((BroadcastDestination *)ARR_GetElement(broadcasts, i))->local_addr.sock_fd);

  ARR_DestroyInstance(broadcasts);
  ADF_DestroyTable(access_auth_table);
}

/* ================================================== */

static void
restart_timeout(NCR_Instance inst, double delay)
{
  /* Check if we can transmit */
  if (inst->tx_suspended) {
    assert(!inst->tx_timeout_id);
    return;
  }

  /* Stop both rx and tx timers if running */
  SCH_RemoveTimeout(inst->rx_timeout_id);
  inst->rx_timeout_id = 0;
  SCH_RemoveTimeout(inst->tx_timeout_id);

  /* Start new timer for transmission */
  inst->tx_timeout_id = SCH_AddTimeoutInClass(delay, get_separation(inst->local_poll),
                                              SAMPLING_RANDOMNESS,
                                              inst->mode == MODE_CLIENT ?
                                                SCH_NtpClientClass : SCH_NtpPeerClass,
                                              transmit_timeout, (void *)inst);
}

/* ================================================== */

static void
start_initial_timeout(NCR_Instance inst)
{
  double delay, last_tx;
  struct timespec now;

  if (!inst->tx_timeout_id) {
    /* This will be the first transmission after mode change */

    /* Mark source active */
    SRC_SetActive(inst->source);
  }

  /* In case the offline period was too short, adjust the delay to keep
     the interval between packets at least as long as the current polling
     interval */
  SCH_GetLastEventTime(&now, NULL, NULL);
  last_tx = UTI_DiffTimespecsToDouble(&now, &inst->local_tx.ts);
  if (last_tx < 0.0)
    last_tx = 0.0;
  delay = get_transmit_delay(inst, 0, 0.0) - last_tx;
  if (delay < INITIAL_DELAY)
    delay = INITIAL_DELAY;

  restart_timeout(inst, delay);
}

/* ================================================== */

static void
close_client_socket(NCR_Instance inst)
{
  if (inst->mode == MODE_CLIENT && inst->local_addr.sock_fd != INVALID_SOCK_FD) {
    NIO_CloseClientSocket(inst->local_addr.sock_fd);
    inst->local_addr.sock_fd = INVALID_SOCK_FD;
  }

  SCH_RemoveTimeout(inst->rx_timeout_id);
  inst->rx_timeout_id = 0;
}

/* ================================================== */

static void
take_offline(NCR_Instance inst)
{
  inst->opmode = MD_OFFLINE;

  SCH_RemoveTimeout(inst->tx_timeout_id);
  inst->tx_timeout_id = 0;

  /* Mark source unreachable */
  SRC_ResetReachability(inst->source);

  /* And inactive */
  SRC_UnsetActive(inst->source);

  close_client_socket(inst);

  NCR_ResetInstance(inst);
}

/* ================================================== */

NCR_Instance
NCR_GetInstance(NTP_Remote_Address *remote_addr, NTP_Source_Type type, SourceParameters *params)
{
  NCR_Instance result;

  result = MallocNew(struct NCR_Instance_Record);

  result->remote_addr = *remote_addr;
  result->local_addr.ip_addr.family = IPADDR_UNSPEC;
  result->local_addr.if_index = INVALID_IF_INDEX;

  switch (type) {
    case NTP_SERVER:
      /* Client socket will be obtained when sending request */
      result->local_addr.sock_fd = INVALID_SOCK_FD;
      result->mode = MODE_CLIENT;
      break;
    case NTP_PEER:
      result->local_addr.sock_fd = NIO_OpenServerSocket(remote_addr);
      result->mode = MODE_ACTIVE;
      break;
    default:
      assert(0);
  }

  result->interleaved = params->interleaved;

  result->minpoll = params->minpoll;
  if (result->minpoll < MIN_POLL)
    result->minpoll = SRC_DEFAULT_MINPOLL;
  else if (result->minpoll > MAX_POLL)
    result->minpoll = MAX_POLL;

  result->maxpoll = params->maxpoll;
  if (result->maxpoll < MIN_POLL)
    result->maxpoll = SRC_DEFAULT_MAXPOLL;
  else if (result->maxpoll > MAX_POLL)
    result->maxpoll = MAX_POLL;
  if (result->maxpoll < result->minpoll)
    result->maxpoll = result->minpoll;

  result->min_stratum = params->min_stratum;
  if (result->min_stratum >= NTP_MAX_STRATUM)
    result->min_stratum = NTP_MAX_STRATUM - 1;

  /* Presend doesn't work in symmetric mode */
  result->presend_minpoll = params->presend_minpoll;
  if (result->presend_minpoll <= MAX_POLL && result->mode != MODE_CLIENT)
    result->presend_minpoll = MAX_POLL + 1;

  result->max_delay = CLAMP(0.0, params->max_delay, MAX_MAXDELAY);
  result->max_delay_ratio = CLAMP(0.0, params->max_delay_ratio, MAX_MAXDELAYRATIO);
  result->max_delay_dev_ratio = CLAMP(0.0, params->max_delay_dev_ratio, MAX_MAXDELAYDEVRATIO);
  result->offset_correction = params->offset;
  result->auto_burst = params->burst;
  result->auto_offline = params->auto_offline;
  result->poll_target = params->poll_target;

  result->version = NTP_VERSION;

  if (params->authkey == INACTIVE_AUTHKEY) {
    result->auth_mode = AUTH_NONE;
    result->auth_key_id = 0;
  } else {
    result->auth_mode = AUTH_SYMMETRIC;
    result->auth_key_id = params->authkey;
    if (!KEY_KeyKnown(result->auth_key_id)) {
      LOG(LOGS_WARN, "Key %"PRIu32" used by source %s is %s",
          result->auth_key_id, UTI_IPToString(&result->remote_addr.ip_addr),
          "missing");
    } else if (!KEY_CheckKeyLength(result->auth_key_id)) {
      LOG(LOGS_WARN, "Key %"PRIu32" used by source %s is %s",
          result->auth_key_id, UTI_IPToString(&result->remote_addr.ip_addr),
          "too short");
    }

    /* If the MAC in NTPv4 packets would be truncated, use version 3 by
       default for compatibility with older chronyd servers */
    if (KEY_GetAuthLength(result->auth_key_id) + 4 > NTP_MAX_V4_MAC_LENGTH)
      result->version = 3;
  }

  if (params->version)
    result->version = CLAMP(NTP_MIN_COMPAT_VERSION, params->version, NTP_VERSION);

  /* Create a source instance for this NTP source */
  result->source = SRC_CreateNewInstance(UTI_IPToRefid(&remote_addr->ip_addr),
                                         SRC_NTP, params->sel_options,
                                         &result->remote_addr.ip_addr,
                                         params->min_samples, params->max_samples,
                                         params->min_delay, params->asymmetry);

  if (params->filter_length >= 1)
    result->filter = SPF_CreateInstance(params->filter_length, params->filter_length,
                                        NTP_MAX_DISPERSION, 0.0);
  else
    result->filter = NULL;

  result->rx_timeout_id = 0;
  result->tx_timeout_id = 0;
  result->tx_suspended = 1;
  result->opmode = params->connectivity == SRC_ONLINE ||
                   (params->connectivity == SRC_MAYBE_ONLINE &&
                    NIO_IsServerConnectable(remote_addr)) ? MD_ONLINE : MD_OFFLINE;
  result->local_poll = result->minpoll;
  result->poll_score = 0.0;
  zero_local_timestamp(&result->local_tx);
  result->burst_good_samples_to_go = 0;
  result->burst_total_samples_to_go = 0;
  memset(&result->report, 0, sizeof (result->report));
  
  NCR_ResetInstance(result);

  if (params->iburst) {
    NCR_InitiateSampleBurst(result, IBURST_GOOD_SAMPLES, IBURST_TOTAL_SAMPLES);
  }

  return result;
}

/* ================================================== */

/* Destroy an instance */
void
NCR_DestroyInstance(NCR_Instance instance)
{
  if (instance->opmode != MD_OFFLINE)
    take_offline(instance);

  if (instance->mode == MODE_ACTIVE)
    NIO_CloseServerSocket(instance->local_addr.sock_fd);

  if (instance->filter)
    SPF_DestroyInstance(instance->filter);

  /* This will destroy the source instance inside the
     structure, which will cause reselection if this was the
     synchronising source etc. */
  SRC_DestroyInstance(instance->source);

  /* Free the data structure */
  Free(instance);
}

/* ================================================== */

void
NCR_StartInstance(NCR_Instance instance)
{
  instance->tx_suspended = 0;
  if (instance->opmode != MD_OFFLINE)
    start_initial_timeout(instance);
}

/* ================================================== */

void
NCR_ResetInstance(NCR_Instance instance)
{
  instance->tx_count = 0;
  instance->presend_done = 0;

  instance->remote_poll = 0;
  instance->remote_stratum = 0;

  instance->valid_rx = 0;
  instance->valid_timestamps = 0;
  UTI_ZeroNtp64(&instance->remote_ntp_rx);
  UTI_ZeroNtp64(&instance->remote_ntp_tx);
  UTI_ZeroNtp64(&instance->local_ntp_rx);
  UTI_ZeroNtp64(&instance->local_ntp_tx);
  zero_local_timestamp(&instance->local_rx);

  zero_local_timestamp(&instance->prev_local_tx);
  instance->prev_local_poll = 0;
  instance->prev_tx_count = 0;

  instance->updated_init_timestamps = 0;
  UTI_ZeroNtp64(&instance->init_remote_ntp_tx);
  zero_local_timestamp(&instance->init_local_rx);

  if (instance->filter)
    SPF_DropSamples(instance->filter);
}

/* ================================================== */

void
NCR_ResetPoll(NCR_Instance instance)
{
  if (instance->local_poll != instance->minpoll) {
    instance->local_poll = instance->minpoll;

    /* The timer was set with a longer poll interval, restart it */
    if (instance->tx_timeout_id)
      restart_timeout(instance, get_transmit_delay(instance, 0, 0.0));
  }
}

/* ================================================== */

void
NCR_ChangeRemoteAddress(NCR_Instance inst, NTP_Remote_Address *remote_addr)
{
  memset(&inst->report, 0, sizeof (inst->report));
  NCR_ResetInstance(inst);
  inst->remote_addr = *remote_addr;

  if (inst->mode == MODE_CLIENT)
    close_client_socket(inst);
  else {
    NIO_CloseServerSocket(inst->local_addr.sock_fd);
    inst->local_addr.ip_addr.family = IPADDR_UNSPEC;
    inst->local_addr.if_index = INVALID_IF_INDEX;
    inst->local_addr.sock_fd = NIO_OpenServerSocket(remote_addr);
  }

  /* Update the reference ID and reset the source/sourcestats instances */
  SRC_SetRefid(inst->source, UTI_IPToRefid(&remote_addr->ip_addr),
               &inst->remote_addr.ip_addr);
  SRC_ResetInstance(inst->source);
}

/* ================================================== */

static void
adjust_poll(NCR_Instance inst, double adj)
{
  inst->poll_score += adj;

  if (inst->poll_score >= 1.0) {
    inst->local_poll += (int)inst->poll_score;
    inst->poll_score -= (int)inst->poll_score;
  }

  if (inst->poll_score < 0.0) {
    inst->local_poll += (int)(inst->poll_score - 1.0);
    inst->poll_score -= (int)(inst->poll_score - 1.0);
  }
  
  /* Clamp polling interval to defined range */
  if (inst->local_poll < inst->minpoll) {
    inst->local_poll = inst->minpoll;
    inst->poll_score = 0;
  } else if (inst->local_poll > inst->maxpoll) {
    inst->local_poll = inst->maxpoll;
    inst->poll_score = 1.0;
  }

  /* Don't allow a sub-second polling interval if the source is not reachable
     or it is not in a local network according to the measured delay */
  if (inst->local_poll < MIN_NONLAN_POLL &&
      (!SRC_IsReachable(inst->source) ||
       SST_MinRoundTripDelay(SRC_GetSourcestats(inst->source)) > MAX_LAN_PEER_DELAY))
    inst->local_poll = MIN_NONLAN_POLL;
}

/* ================================================== */

static double
get_poll_adj(NCR_Instance inst, double error_in_estimate, double peer_distance)
{
  double poll_adj;
  int samples;

  if (error_in_estimate > peer_distance) {
    /* If the prediction is not even within +/- the peer distance of the peer,
       we are clearly not tracking the peer at all well, so we back off the
       sampling rate depending on just how bad the situation is */
    poll_adj = -log(error_in_estimate / peer_distance) / log(2.0);
  } else {
    samples = SST_Samples(SRC_GetSourcestats(inst->source));

    /* Adjust polling interval so that the number of sourcestats samples
       remains close to the target value */
    poll_adj = ((double)samples / inst->poll_target - 1.0) / inst->poll_target;

    /* Make interval shortening quicker */
    if (samples < inst->poll_target) {
      poll_adj *= 2.0;
    }
  }

  return poll_adj;
}

/* ================================================== */

static int
get_transmit_poll(NCR_Instance inst)
{
  int poll;

  poll = inst->local_poll;

  /* In symmetric mode, if the peer is responding, use shorter of the local
     and remote poll interval, but not shorter than the minimum */
  if (inst->mode == MODE_ACTIVE && poll > inst->remote_poll &&
      SRC_IsReachable(inst->source))
    poll = MAX(inst->remote_poll, inst->minpoll);

  return poll;
}

/* ================================================== */

static double
get_transmit_delay(NCR_Instance inst, int on_tx, double last_tx)
{
  int poll_to_use, stratum_diff;
  double delay_time;

  /* If we're in burst mode, queue for immediate dispatch.

     If we're operating in client/server mode, queue the timeout for
     the poll interval hence.  The fact that a timeout has been queued
     in the transmit handler is immaterial - that is only done so that
     we at least send something, if no reply is heard.

     If we're in symmetric mode, we have to take account of the peer's
     wishes, otherwise his sampling regime will fall to pieces.  If
     we're in client/server mode, we don't care what poll interval the
     server responded with last time. */

  poll_to_use = get_transmit_poll(inst);
  delay_time = UTI_Log2ToDouble(poll_to_use);

  switch (inst->opmode) {
    case MD_OFFLINE:
      assert(0);
      break;
    case MD_ONLINE:
      switch(inst->mode) {
        case MODE_CLIENT:
          if (inst->presend_done)
            delay_time = WARM_UP_DELAY;
          break;

        case MODE_ACTIVE:
          /* If the remote stratum is higher than ours, wait a bit for the next
             packet before responding in order to minimize the delay of the
             measurement and its error for the peer which has higher stratum.
             If the remote stratum is equal to ours, try to interleave packets
             evenly with the peer. */
          stratum_diff = inst->remote_stratum - REF_GetOurStratum();
          if ((stratum_diff > 0 && last_tx * PEER_SAMPLING_ADJ < delay_time) ||
              (!on_tx && !stratum_diff &&
               last_tx / delay_time > PEER_SAMPLING_ADJ - 0.5))
            delay_time *= PEER_SAMPLING_ADJ;

          /* Substract the already spend time */
          if (last_tx > 0.0)
            delay_time -= last_tx;
          if (delay_time < 0.0)
            delay_time = 0.0;

          break;
        default:
          assert(0);
          break;
      }
      break;

    case MD_BURST_WAS_ONLINE:
    case MD_BURST_WAS_OFFLINE:
      /* Burst modes */
      delay_time = MIN(MAX_BURST_INTERVAL, MAX_BURST_POLL_RATIO * delay_time);
      break;
    default:
      assert(0);
      break;
  }

  return delay_time;
}

/* ================================================== */
/* Calculate sampling separation for given polling interval */

static double
get_separation(int poll)
{
  double separation;

  assert(poll >= MIN_POLL && poll <= MAX_POLL);

  /* Allow up to 8 sources using the same short interval to not be limited
     by the separation */
  separation = UTI_Log2ToDouble(poll - 3);

  return CLAMP(MIN_SAMPLING_SEPARATION, separation, MAX_SAMPLING_SEPARATION);
}

/* ================================================== */
/* Timeout handler for closing the client socket when no acceptable
   reply can be received from the server */

static void
receive_timeout(void *arg)
{
  NCR_Instance inst = (NCR_Instance)arg;

  DEBUG_LOG("Receive timeout for [%s:%d]",
            UTI_IPToString(&inst->remote_addr.ip_addr), inst->remote_addr.port);

  inst->rx_timeout_id = 0;
  close_client_socket(inst);
}

/* ================================================== */

static int
transmit_packet(NTP_Mode my_mode, /* The mode this machine wants to be */
                int interleaved, /* Flag enabling interleaved mode */
                int my_poll, /* The log2 of the local poll interval */
                int version, /* The NTP version to be set in the packet */
                int auth_mode, /* The authentication mode */
                uint32_t key_id, /* The authentication key ID */
                NTP_int64 *remote_ntp_rx, /* The receive timestamp from received packet */
                NTP_int64 *remote_ntp_tx, /* The transmit timestamp from received packet */
                NTP_Local_Timestamp *local_rx, /* The RX time of the received packet */
                NTP_Local_Timestamp *local_tx, /* The TX time of the previous packet
                                                  RESULT : TX time of this packet */
                NTP_int64 *local_ntp_rx, /* The receive timestamp from the previous packet
                                            RESULT : receive timestamp from this packet */
                NTP_int64 *local_ntp_tx, /* The transmit timestamp from the previous packet
                                            RESULT : transmit timestamp from this packet */
                NTP_Remote_Address *where_to, /* Where to address the reponse to */
                NTP_Local_Address *from /* From what address to send it */
                )
{
  NTP_Packet message;
  int auth_len, max_auth_len, length, ret, precision;
  struct timespec local_receive, local_transmit;
  double smooth_offset, local_transmit_err;
  NTP_int64 ts_fuzz;

  /* Parameters read from reference module */
  int are_we_synchronised, our_stratum, smooth_time;
  NTP_Leap leap_status;
  uint32_t our_ref_id;
  struct timespec our_ref_time;
  double our_root_delay, our_root_dispersion;

  /* Don't reply with version higher than ours */
  if (version > NTP_VERSION) {
    version = NTP_VERSION;
  }

  /* Check if the packet can be formed in the interleaved mode */
  if (interleaved && (!remote_ntp_rx || !local_tx || UTI_IsZeroTimespec(&local_tx->ts)))
    interleaved = 0;

  smooth_time = 0;
  smooth_offset = 0.0;

  if (my_mode == MODE_CLIENT) {
    /* Don't reveal local time or state of the clock in client packets */
    precision = 32;
    leap_status = our_stratum = our_ref_id = 0;
    our_root_delay = our_root_dispersion = 0.0;
    UTI_ZeroTimespec(&our_ref_time);
  } else {
    /* This is accurate enough and cheaper than calling LCL_ReadCookedTime.
       A more accurate timestamp will be taken later in this function. */
    SCH_GetLastEventTime(&local_transmit, NULL, NULL);

    REF_GetReferenceParams(&local_transmit,
                           &are_we_synchronised, &leap_status,
                           &our_stratum,
                           &our_ref_id, &our_ref_time,
                           &our_root_delay, &our_root_dispersion);

    /* Get current smoothing offset when sending packet to a client */
    if (SMT_IsEnabled() && (my_mode == MODE_SERVER || my_mode == MODE_BROADCAST)) {
      smooth_offset = SMT_GetOffset(&local_transmit);
      smooth_time = fabs(smooth_offset) > LCL_GetSysPrecisionAsQuantum();

      /* Suppress leap second when smoothing and slew mode are enabled */
      if (REF_GetLeapMode() == REF_LeapModeSlew &&
          (leap_status == LEAP_InsertSecond || leap_status == LEAP_DeleteSecond))
        leap_status = LEAP_Normal;
    }

    precision = LCL_GetSysPrecisionAsLog();
  }

  if (smooth_time && !UTI_IsZeroTimespec(&local_rx->ts)) {
    our_ref_id = NTP_REFID_SMOOTH;
    UTI_AddDoubleToTimespec(&our_ref_time, smooth_offset, &our_ref_time);
    UTI_AddDoubleToTimespec(&local_rx->ts, smooth_offset, &local_receive);
  } else {
    local_receive = local_rx->ts;
  }

  /* Generate transmit packet */
  message.lvm = NTP_LVM(leap_status, version, my_mode);
  /* Stratum 16 and larger are invalid */
  if (our_stratum < NTP_MAX_STRATUM) {
    message.stratum = our_stratum;
  } else {
    message.stratum = NTP_INVALID_STRATUM;
  }
 
  message.poll = my_poll;
  message.precision = precision;

  /* If we're sending a client mode packet and we aren't synchronized yet, 
     we might have to set up artificial values for some of these parameters */
  message.root_delay = UTI_DoubleToNtp32(our_root_delay);
  message.root_dispersion = UTI_DoubleToNtp32(our_root_dispersion);

  message.reference_id = htonl(our_ref_id);

  /* Now fill in timestamps */

  UTI_TimespecToNtp64(&our_ref_time, &message.reference_ts, NULL);

  /* Don't reveal timestamps which are not necessary for the protocol */

  if (my_mode != MODE_CLIENT || interleaved) {
    /* Originate - this comes from the last packet the source sent us */
    message.originate_ts = interleaved ? *remote_ntp_rx : *remote_ntp_tx;

    do {
      /* Prepare random bits which will be added to the receive timestamp */
      UTI_GetNtp64Fuzz(&ts_fuzz, precision);

      /* Receive - this is when we received the last packet from the source.
         This timestamp will have been adjusted so that it will now look to
         the source like we have been running on our latest estimate of
         frequency all along */
      UTI_TimespecToNtp64(&local_receive, &message.receive_ts, &ts_fuzz);

      /* Do not send a packet with a non-zero receive timestamp equal to the
         originate timestamp or previous receive timestamp */
    } while (!UTI_IsZeroNtp64(&message.receive_ts) &&
             UTI_IsEqualAnyNtp64(&message.receive_ts, &message.originate_ts,
                                 local_ntp_rx, NULL));
  } else {
    UTI_ZeroNtp64(&message.originate_ts);
    UTI_ZeroNtp64(&message.receive_ts);
  }

  do {
    /* Prepare random bits which will be added to the transmit timestamp */
    UTI_GetNtp64Fuzz(&ts_fuzz, precision);

    /* Transmit - this our local time right now!  Also, we might need to
       store this for our own use later, next time we receive a message
       from the source we're sending to now. */
    LCL_ReadCookedTime(&local_transmit, &local_transmit_err);

    if (smooth_time)
      UTI_AddDoubleToTimespec(&local_transmit, smooth_offset, &local_transmit);

    length = NTP_NORMAL_PACKET_LENGTH;

    /* Authenticate the packet */

    if (auth_mode == AUTH_SYMMETRIC || auth_mode == AUTH_MSSNTP) {
      /* Pre-compensate the transmit time by approximately how long it will
         take to generate the authentication data */
      local_transmit.tv_nsec += auth_mode == AUTH_SYMMETRIC ?
                                KEY_GetAuthDelay(key_id) : NSD_GetAuthDelay(key_id);
      UTI_NormaliseTimespec(&local_transmit);
      UTI_TimespecToNtp64(interleaved ? &local_tx->ts : &local_transmit,
                          &message.transmit_ts, &ts_fuzz);

      if (auth_mode == AUTH_SYMMETRIC) {
        /* Truncate long MACs in NTPv4 packets to allow deterministic parsing
           of extension fields (RFC 7822) */
        max_auth_len = version == 4 ?
                       NTP_MAX_V4_MAC_LENGTH - 4 : sizeof (message.auth_data);

        auth_len = KEY_GenerateAuth(key_id, (unsigned char *) &message,
                                    offsetof(NTP_Packet, auth_keyid),
                                    (unsigned char *)&message.auth_data, max_auth_len);
        if (!auth_len) {
          DEBUG_LOG("Could not generate auth data with key %"PRIu32, key_id);
          return 0;
        }

        message.auth_keyid = htonl(key_id);
        length += sizeof (message.auth_keyid) + auth_len;
      } else if (auth_mode == AUTH_MSSNTP) {
        /* MS-SNTP packets are signed (asynchronously) by ntp_signd */
        return NSD_SignAndSendPacket(key_id, &message, where_to, from, length);
      }
    } else {
      UTI_TimespecToNtp64(interleaved ? &local_tx->ts : &local_transmit,
                          &message.transmit_ts, &ts_fuzz);
    }

    /* Do not send a packet with a non-zero transmit timestamp which is
       equal to any of the following timestamps:
       - receive (to allow reliable detection of the interleaved mode)
       - originate (to prevent the packet from being its own valid response
                    in the symmetric mode)
       - previous transmit (to invalidate responses to the previous packet)
       (the precision must be at least -30 to prevent an infinite loop!) */
  } while (!UTI_IsZeroNtp64(&message.transmit_ts) &&
           UTI_IsEqualAnyNtp64(&message.transmit_ts, &message.receive_ts,
                               &message.originate_ts, local_ntp_tx));

  ret = NIO_SendPacket(&message, where_to, from, length, local_tx != NULL);

  if (local_tx) {
    local_tx->ts = local_transmit;
    local_tx->err = local_transmit_err;
    local_tx->source = NTP_TS_DAEMON;
  }

  if (local_ntp_rx)
    *local_ntp_rx = message.receive_ts;
  if (local_ntp_tx)
    *local_ntp_tx = message.transmit_ts;

  return ret;
}

/* ================================================== */
/* Timeout handler for transmitting to a source. */

static void
transmit_timeout(void *arg)
{
  NCR_Instance inst = (NCR_Instance) arg;
  NTP_Local_Address local_addr;
  int interleaved, initial, sent;

  inst->tx_timeout_id = 0;

  switch (inst->opmode) {
    case MD_BURST_WAS_ONLINE:
      /* With online burst switch to online before last packet */
      if (inst->burst_total_samples_to_go <= 1)
        inst->opmode = MD_ONLINE;
      break;
    case MD_BURST_WAS_OFFLINE:
      if (inst->burst_total_samples_to_go <= 0)
        take_offline(inst);
      break;
    case MD_ONLINE:
      /* Start a new burst if the burst option is enabled and the average
         polling interval including the burst will not fall below the
         minimum polling interval */
      if (inst->auto_burst && inst->local_poll > inst->minpoll)
        NCR_InitiateSampleBurst(inst, BURST_GOOD_SAMPLES,
                                MIN(1 << (inst->local_poll - inst->minpoll),
                                    MAX_BURST_TOTAL_SAMPLES));
      break;
    default:
      break;
  }

  if (inst->opmode == MD_OFFLINE) {
    return;
  }

  DEBUG_LOG("Transmit timeout for [%s:%d]",
      UTI_IPToString(&inst->remote_addr.ip_addr), inst->remote_addr.port);

  /* Open new client socket */
  if (inst->mode == MODE_CLIENT) {
    close_client_socket(inst);
    assert(inst->local_addr.sock_fd == INVALID_SOCK_FD);
    inst->local_addr.sock_fd = NIO_OpenClientSocket(&inst->remote_addr);
  }

  /* Don't require the packet to be sent from the same address as before */
  local_addr.ip_addr.family = IPADDR_UNSPEC;
  local_addr.if_index = INVALID_IF_INDEX;
  local_addr.sock_fd = inst->local_addr.sock_fd;

  /* In symmetric mode, don't send a packet in interleaved mode unless it
     is the first response to the last valid request received from the peer
     and there was just one response to the previous valid request.  This
     prevents the peer from matching the transmit timestamp with an older
     response if it can't detect missed responses.  In client mode, which has
     at most one response per request, check how many responses are missing to
     prevent the server from responding with a very old transmit timestamp. */
  interleaved = inst->interleaved &&
                ((inst->mode == MODE_CLIENT &&
                  inst->tx_count < MAX_CLIENT_INTERLEAVED_TX) ||
                 (inst->mode == MODE_ACTIVE &&
                  inst->prev_tx_count == 1 && inst->tx_count == 0));

  /* In symmetric mode, if no valid response was received since the previous
     transmission, respond to the last received packet even if it failed some
     specific NTP tests.  This is necessary for starting and restarting the
     protocol, e.g. when a packet was lost. */
  initial = inst->mode == MODE_ACTIVE && !inst->valid_rx &&
            !UTI_IsZeroNtp64(&inst->init_remote_ntp_tx);

  /* Prepare for the response */
  inst->valid_rx = 0;
  inst->updated_init_timestamps = 0;
  if (initial)
    inst->valid_timestamps = 0;

  /* Check whether we need to 'warm up' the link to the other end by
     sending an NTP exchange to ensure both ends' ARP caches are
     primed or whether we need to send two packets first to ensure a
     server in the interleaved mode has a fresh timestamp for us. */
  if (inst->presend_minpoll <= inst->local_poll && !inst->presend_done &&
      !inst->burst_total_samples_to_go) {
    inst->presend_done = interleaved ? 2 : 1;
  } else if (inst->presend_done > 0) {
    inst->presend_done--;
  }

  /* Send the request (which may also be a response in the symmetric mode) */
  sent = transmit_packet(inst->mode, interleaved, inst->local_poll, inst->version,
                         inst->auth_mode, inst->auth_key_id,
                         initial ? NULL : &inst->remote_ntp_rx,
                         initial ? &inst->init_remote_ntp_tx : &inst->remote_ntp_tx,
                         initial ? &inst->init_local_rx : &inst->local_rx,
                         &inst->local_tx, &inst->local_ntp_rx, &inst->local_ntp_tx,
                         &inst->remote_addr, &local_addr);

  ++inst->tx_count;
  if (sent)
    inst->report.total_tx_count++;

  /* If the source loses connectivity and our packets are still being sent,
     back off the sampling rate to reduce the network traffic.  If it's the
     source to which we are currently locked, back off slowly. */

  if (inst->tx_count >= 2) {
    /* Implies we have missed at least one transmission */

    if (sent) {
      adjust_poll(inst, SRC_IsSyncPeer(inst->source) ? 0.1 : 0.25);
    }

    SRC_UpdateReachability(inst->source, 0);
  }

  /* With auto_offline take the source offline if sending failed */
  if (!sent && inst->auto_offline)
    NCR_SetConnectivity(inst, SRC_OFFLINE);

  switch (inst->opmode) {
    case MD_BURST_WAS_ONLINE:
      /* When not reachable, don't stop online burst until sending succeeds */
      if (!sent && !SRC_IsReachable(inst->source))
        break;
      /* Fall through */
    case MD_BURST_WAS_OFFLINE:
      --inst->burst_total_samples_to_go;
      break;
    case MD_OFFLINE:
      return;
    default:
      break;
  }

  /* Restart timer for this message */
  restart_timeout(inst, get_transmit_delay(inst, 1, 0.0));

  /* If a client packet was just sent, schedule a timeout to close the socket
     at the time when all server replies would fail the delay test, so the
     socket is not open for longer than necessary */
  if (inst->mode == MODE_CLIENT)
    inst->rx_timeout_id = SCH_AddTimeoutByDelay(inst->max_delay + MAX_SERVER_INTERVAL,
                                                receive_timeout, (void *)inst);
}

/* ================================================== */

static int
check_packet_format(NTP_Packet *message, int length)
{
  int version;

  /* Check version and length */

  version = NTP_LVM_TO_VERSION(message->lvm);
  if (version < NTP_MIN_COMPAT_VERSION || version > NTP_MAX_COMPAT_VERSION) {
    DEBUG_LOG("NTP packet has invalid version %d", version);
    return 0;
  } 

  if (length < NTP_NORMAL_PACKET_LENGTH || (unsigned int)length % 4) {
    DEBUG_LOG("NTP packet has invalid length %d", length);
    return 0;
  }

  /* We can't reliably check the packet for invalid extension fields as we
     support MACs longer than the shortest valid extension field */

  return 1;
}

/* ================================================== */

static int
is_zero_data(unsigned char *data, int length)
{
  int i;

  for (i = 0; i < length; i++)
    if (data[i])
      return 0;
  return 1;
}

/* ================================================== */

static int
check_packet_auth(NTP_Packet *pkt, int length,
                  AuthenticationMode *auth_mode, uint32_t *key_id)
{
  int i, version, remainder, ext_length, max_mac_length;
  unsigned char *data;
  uint32_t id;

  /* Go through extension fields and see if there is a valid MAC */

  version = NTP_LVM_TO_VERSION(pkt->lvm);
  i = NTP_NORMAL_PACKET_LENGTH;
  data = (void *)pkt;

  while (1) {
    remainder = length - i;

    /* Check if the remaining data is a valid MAC.  There is a limit on MAC
       length in NTPv4 packets to allow deterministic parsing of extension
       fields (RFC 7822), but we need to support longer MACs to not break
       compatibility with older chrony clients.  This needs to be done before
       trying to parse the data as an extension field. */

    max_mac_length = version == 4 && remainder <= NTP_MAX_V4_MAC_LENGTH ?
                     NTP_MAX_V4_MAC_LENGTH : NTP_MAX_MAC_LENGTH;

    if (remainder >= NTP_MIN_MAC_LENGTH && remainder <= max_mac_length) {
      id = ntohl(*(uint32_t *)(data + i));
      if (KEY_CheckAuth(id, (void *)pkt, i, (void *)(data + i + 4),
                        remainder - 4, max_mac_length - 4)) {
        *auth_mode = AUTH_SYMMETRIC;
        *key_id = id;

        /* If it's an NTPv4 packet with long MAC and no extension fields,
           rewrite the version in the packet to respond with long MAC too */
        if (version == 4 && NTP_NORMAL_PACKET_LENGTH + remainder == length &&
            remainder > NTP_MAX_V4_MAC_LENGTH)
          pkt->lvm = NTP_LVM(NTP_LVM_TO_LEAP(pkt->lvm), 3, NTP_LVM_TO_MODE(pkt->lvm));

        return 1;
      }
    }

    /* Check if this is a valid NTPv4 extension field and skip it.  It should
       have a 16-bit type, 16-bit length, and data padded to 32 bits. */
    if (version == 4 && remainder >= NTP_MIN_EXTENSION_LENGTH) {
      ext_length = ntohs(*(uint16_t *)(data + i + 2));
      if (ext_length >= NTP_MIN_EXTENSION_LENGTH &&
          ext_length <= remainder && ext_length % 4 == 0) {
        i += ext_length;
        continue;
      }
    }

    /* Invalid or missing MAC, or format error */
    break;
  }

  /* This is not 100% reliable as a MAC could fail to authenticate and could
     pass as an extension field, leaving reminder smaller than the minimum MAC
     length */
  if (remainder >= NTP_MIN_MAC_LENGTH) {
    *auth_mode = AUTH_SYMMETRIC;
    *key_id = ntohl(*(uint32_t *)(data + i));

    /* Check if it is an MS-SNTP authenticator field or extended authenticator
       field with zeroes as digest */
    if (version == 3 && *key_id) {
      if (remainder == 20 && is_zero_data(data + i + 4, remainder - 4))
        *auth_mode = AUTH_MSSNTP;
      else if (remainder == 72 && is_zero_data(data + i + 8, remainder - 8))
        *auth_mode = AUTH_MSSNTP_EXT;
    }
  } else {
    *auth_mode = AUTH_NONE;
    *key_id = 0;
  }

  return 0;
}

/* ================================================== */

static int
check_delay_ratio(NCR_Instance inst, SST_Stats stats,
                struct timespec *sample_time, double delay)
{
  double last_sample_ago, predicted_offset, min_delay, skew, std_dev;
  double max_delay;

  if (inst->max_delay_ratio < 1.0 ||
      !SST_GetDelayTestData(stats, sample_time, &last_sample_ago,
                            &predicted_offset, &min_delay, &skew, &std_dev))
    return 1;

  max_delay = min_delay * inst->max_delay_ratio +
              last_sample_ago * (skew + LCL_GetMaxClockError());

  if (delay <= max_delay)
    return 1;

  DEBUG_LOG("maxdelayratio: delay=%e max_delay=%e", delay, max_delay);
  return 0;
}

/* ================================================== */

static int
check_delay_dev_ratio(NCR_Instance inst, SST_Stats stats,
                      struct timespec *sample_time, double offset, double delay)
{
  double last_sample_ago, predicted_offset, min_delay, skew, std_dev;
  double delta, max_delta, error_in_estimate;

  if (!SST_GetDelayTestData(stats, sample_time, &last_sample_ago,
                            &predicted_offset, &min_delay, &skew, &std_dev))
    return 1;

  /* Require that the ratio of the increase in delay from the minimum to the
     standard deviation is less than max_delay_dev_ratio.  In the allowed
     increase in delay include also dispersion. */

  max_delta = std_dev * inst->max_delay_dev_ratio +
              last_sample_ago * (skew + LCL_GetMaxClockError());
  delta = (delay - min_delay) / 2.0;

  if (delta <= max_delta)
    return 1;

  error_in_estimate = offset + predicted_offset;

  /* Before we decide to drop the sample, make sure the difference between
     measured offset and predicted offset is not significantly larger than
     the increase in delay */
  if (fabs(error_in_estimate) - delta > max_delta)
    return 1;

  DEBUG_LOG("maxdelaydevratio: error=%e delay=%e delta=%e max_delta=%e",
            error_in_estimate, delay, delta, max_delta);
  return 0;
}

/* ================================================== */

static void
process_sample(NCR_Instance inst, NTP_Sample *sample)
{
  double estimated_offset, error_in_estimate, filtered_sample_ago;
  NTP_Sample filtered_sample;
  int filtered_samples;

  /* Accumulate the sample to the median filter if it is enabled.  When the
     filter produces a result, check if it is not too old, i.e. the filter did
     not miss too many samples due to missing responses or failing tests. */
  if (inst->filter) {
    SPF_AccumulateSample(inst->filter, sample);

    filtered_samples = SPF_GetNumberOfSamples(inst->filter);

    if (!SPF_GetFilteredSample(inst->filter, &filtered_sample))
      return;

    filtered_sample_ago = UTI_DiffTimespecsToDouble(&sample->time, &filtered_sample.time);

    if (filtered_sample_ago > SOURCE_REACH_BITS / 2 * filtered_samples *
                              UTI_Log2ToDouble(inst->local_poll)) {
      DEBUG_LOG("filtered sample dropped ago=%f poll=%d", filtered_sample_ago,
                inst->local_poll);
      return;
    }

    sample = &filtered_sample;
  }

  /* Get the estimated offset predicted from previous samples.  The
     convention here is that positive means local clock FAST of
     reference, i.e. backwards to the way that 'offset' is defined. */
  estimated_offset = SST_PredictOffset(SRC_GetSourcestats(inst->source), &sample->time);

  error_in_estimate = fabs(-sample->offset - estimated_offset);

  SRC_AccumulateSample(inst->source, sample);
  SRC_SelectSource(inst->source);

  adjust_poll(inst, get_poll_adj(inst, error_in_estimate,
                                 sample->peer_dispersion + 0.5 * sample->peer_delay));
}

/* ================================================== */

static int
receive_packet(NCR_Instance inst, NTP_Local_Address *local_addr,
               NTP_Local_Timestamp *rx_ts, NTP_Packet *message, int length)
{
  NTP_Sample sample;
  SST_Stats stats;

  int pkt_leap, pkt_version;
  uint32_t pkt_refid, pkt_key_id;
  double pkt_root_delay;
  double pkt_root_dispersion;
  AuthenticationMode pkt_auth_mode;

  /* The skew and estimated frequency offset relative to the remote source */
  double skew, source_freq_lo, source_freq_hi;

  /* RFC 5905 packet tests */
  int test1, test2n, test2i, test2, test3, test5, test6, test7;
  int interleaved_packet, valid_packet, synced_packet;

  /* Additional tests */
  int testA, testB, testC, testD;
  int good_packet;

  /* Kiss-o'-Death codes */
  int kod_rate;

  NTP_Local_Timestamp local_receive, local_transmit;
  double remote_interval, local_interval, response_time;
  double delay_time, precision;
  int updated_timestamps;

  /* ==================== */

  stats = SRC_GetSourcestats(inst->source);

  inst->report.total_rx_count++;

  pkt_leap = NTP_LVM_TO_LEAP(message->lvm);
  pkt_version = NTP_LVM_TO_VERSION(message->lvm);
  pkt_refid = ntohl(message->reference_id);
  pkt_root_delay = UTI_Ntp32ToDouble(message->root_delay);
  pkt_root_dispersion = UTI_Ntp32ToDouble(message->root_dispersion);

  /* Check if the packet is valid per RFC 5905, section 8.
     The test values are 1 when passed and 0 when failed. */
  
  /* Test 1 checks for duplicate packet */
  test1 = UTI_CompareNtp64(&message->receive_ts, &inst->remote_ntp_rx) ||
          UTI_CompareNtp64(&message->transmit_ts, &inst->remote_ntp_tx);

  /* Test 2 checks for bogus packet in the basic and interleaved modes.  This
     ensures the source is responding to the latest packet we sent to it. */
  test2n = !UTI_CompareNtp64(&message->originate_ts, &inst->local_ntp_tx);
  test2i = inst->interleaved &&
           !UTI_CompareNtp64(&message->originate_ts, &inst->local_ntp_rx);
  test2 = test2n || test2i;
  interleaved_packet = !test2n && test2i;
  
  /* Test 3 checks for invalid timestamps.  This can happen when the
     association if not properly 'up'. */
  test3 = !UTI_IsZeroNtp64(&message->originate_ts) &&
          !UTI_IsZeroNtp64(&message->receive_ts) &&
          !UTI_IsZeroNtp64(&message->transmit_ts);

  /* Test 4 would check for denied access.  It would always pass as this
     function is called only for known sources. */

  /* Test 5 checks for authentication failure.  If we expect authenticated info
     from this peer/server and the packet doesn't have it, the authentication
     is bad, or it's authenticated with a different key than expected, it's got
     to fail.  If we don't expect the packet to be authenticated, just ignore
     the test. */
  test5 = inst->auth_mode == AUTH_NONE ||
          (check_packet_auth(message, length, &pkt_auth_mode, &pkt_key_id) &&
           pkt_auth_mode == inst->auth_mode && pkt_key_id == inst->auth_key_id);

  /* Test 6 checks for unsynchronised server */
  test6 = pkt_leap != LEAP_Unsynchronised &&
          message->stratum < NTP_MAX_STRATUM &&
          message->stratum != NTP_INVALID_STRATUM; 

  /* Test 7 checks for bad data.  The root distance must be smaller than a
     defined maximum. */
  test7 = pkt_root_delay / 2.0 + pkt_root_dispersion < NTP_MAX_DISPERSION;

  /* The packet is considered valid if the tests 1-5 passed.  The timestamps
     can be used for synchronisation if the tests 6 and 7 passed too. */
  valid_packet = test1 && test2 && test3 && test5;
  synced_packet = valid_packet && test6 && test7;

  /* Check for Kiss-o'-Death codes */
  kod_rate = 0;
  if (test1 && test2 && test5 && pkt_leap == LEAP_Unsynchronised &&
      message->stratum == NTP_INVALID_STRATUM) {
    if (pkt_refid == KOD_RATE)
      kod_rate = 1;
  }

  if (synced_packet && (!interleaved_packet || inst->valid_timestamps)) {
    /* These are the timespec equivalents of the remote and local epochs */
    struct timespec remote_receive, remote_transmit, remote_request_receive;
    struct timespec local_average, remote_average, prev_remote_transmit;
    double prev_remote_poll_interval;

    /* Select remote and local timestamps for the new sample */
    if (interleaved_packet) {
      /* Prefer previous local TX and remote RX timestamps if it will make
         the intervals significantly shorter in order to improve the accuracy
         of the measured delay */
      if (!UTI_IsZeroTimespec(&inst->prev_local_tx.ts) &&
          MAX_INTERLEAVED_L2L_RATIO *
            UTI_DiffTimespecsToDouble(&inst->local_tx.ts, &inst->local_rx.ts) >
          UTI_DiffTimespecsToDouble(&inst->local_rx.ts, &inst->prev_local_tx.ts)) {
        UTI_Ntp64ToTimespec(&inst->remote_ntp_rx, &remote_receive);
        remote_request_receive = remote_receive;
        local_transmit = inst->prev_local_tx;
      } else {
        UTI_Ntp64ToTimespec(&message->receive_ts, &remote_receive);
        UTI_Ntp64ToTimespec(&inst->remote_ntp_rx, &remote_request_receive);
        local_transmit = inst->local_tx;
      }
      UTI_Ntp64ToTimespec(&message->transmit_ts, &remote_transmit);
      UTI_Ntp64ToTimespec(&inst->remote_ntp_tx, &prev_remote_transmit);
      local_receive = inst->local_rx;
    } else {
      UTI_Ntp64ToTimespec(&message->receive_ts, &remote_receive);
      UTI_Ntp64ToTimespec(&message->transmit_ts, &remote_transmit);
      UTI_ZeroTimespec(&prev_remote_transmit);
      remote_request_receive = remote_receive;
      local_receive = *rx_ts;
      local_transmit = inst->local_tx;
    }

    /* Calculate intervals between remote and local timestamps */
    UTI_AverageDiffTimespecs(&remote_receive, &remote_transmit,
                             &remote_average, &remote_interval);
    UTI_AverageDiffTimespecs(&local_transmit.ts, &local_receive.ts,
                             &local_average, &local_interval);
    response_time = fabs(UTI_DiffTimespecsToDouble(&remote_transmit,
                                                   &remote_request_receive));

    precision = LCL_GetSysPrecisionAsQuantum() + UTI_Log2ToDouble(message->precision);

    /* Calculate delay */
    sample.peer_delay = fabs(local_interval - remote_interval);
    if (sample.peer_delay < precision)
      sample.peer_delay = precision;
    
    /* Calculate offset.  Following the NTP definition, this is negative
       if we are fast of the remote source. */
    sample.offset = UTI_DiffTimespecsToDouble(&remote_average, &local_average);

    /* Apply configured correction */
    sample.offset += inst->offset_correction;

    /* We treat the time of the sample as being midway through the local
       measurement period.  An analysis assuming constant relative
       frequency and zero network delay shows this is the only possible
       choice to estimate the frequency difference correctly for every
       sample pair. */
    sample.time = local_average;
    
    SST_GetFrequencyRange(stats, &source_freq_lo, &source_freq_hi);

    /* Calculate skew */
    skew = (source_freq_hi - source_freq_lo) / 2.0;
    
    /* and then calculate peer dispersion */
    sample.peer_dispersion = MAX(precision, MAX(local_transmit.err, local_receive.err)) +
                             skew * fabs(local_interval);
    
    /* If the source is an active peer, this is the minimum assumed interval
       between previous two transmissions (if not constrained by minpoll) */
    prev_remote_poll_interval = UTI_Log2ToDouble(MIN(inst->remote_poll,
                                                     inst->prev_local_poll));

    /* Additional tests required to pass before accumulating the sample */

    /* Test A requires that the minimum estimate of the peer delay is not
       larger than the configured maximum, in both client modes that the server
       processing time is sane, and in interleaved symmetric mode that the
       measured delay and intervals between remote timestamps don't indicate
       a missed response */
    testA = sample.peer_delay - sample.peer_dispersion <= inst->max_delay &&
            precision <= inst->max_delay &&
            !(inst->mode == MODE_CLIENT && response_time > MAX_SERVER_INTERVAL) &&
            !(inst->mode == MODE_ACTIVE && interleaved_packet &&
              (sample.peer_delay > 0.5 * prev_remote_poll_interval ||
               UTI_CompareNtp64(&message->receive_ts, &message->transmit_ts) <= 0 ||
               (inst->remote_poll <= inst->prev_local_poll &&
                UTI_DiffTimespecsToDouble(&remote_transmit, &prev_remote_transmit) >
                  1.5 * prev_remote_poll_interval)));

    /* Test B requires in client mode that the ratio of the round trip delay
       to the minimum one currently in the stats data register is less than an
       administrator-defined value */
    testB = check_delay_ratio(inst, stats, &sample.time, sample.peer_delay);

    /* Test C requires that the ratio of the increase in delay from the minimum
       one in the stats data register to the standard deviation of the offsets
       in the register is less than an administrator-defined value or the
       difference between measured offset and predicted offset is larger than
       the increase in delay */
    testC = check_delay_dev_ratio(inst, stats, &sample.time, sample.offset, sample.peer_delay);

    /* Test D requires that the remote peer is not synchronised to us to
       prevent a synchronisation loop */
    testD = message->stratum <= 1 || REF_GetMode() != REF_ModeNormal ||
            pkt_refid != UTI_IPToRefid(&local_addr->ip_addr);
  } else {
    remote_interval = local_interval = response_time = 0.0;
    sample.offset = sample.peer_delay = sample.peer_dispersion = 0.0;
    sample.time = rx_ts->ts;
    local_receive = *rx_ts;
    local_transmit = inst->local_tx;
    testA = testB = testC = testD = 0;
  }
  
  /* The packet is considered good for synchronisation if
     the additional tests passed */
  good_packet = testA && testB && testC && testD;

  sample.root_delay = pkt_root_delay + sample.peer_delay;
  sample.root_dispersion = pkt_root_dispersion + sample.peer_dispersion;
  sample.stratum = MAX(message->stratum, inst->min_stratum);
  sample.leap = (NTP_Leap)pkt_leap;

  /* Update the NTP timestamps.  If it's a valid packet from a synchronised
     source, the timestamps may be used later when processing a packet in the
     interleaved mode.  Protect the timestamps against replay attacks in client
     mode, and also in symmetric mode as long as the peers use the same polling
     interval and never start with clocks in future or very distant past.
     The authentication test (test5) is required to prevent DoS attacks using
     unauthenticated packets on authenticated symmetric associations. */
  if ((inst->mode == MODE_CLIENT && valid_packet && !inst->valid_rx) ||
      (inst->mode == MODE_ACTIVE && valid_packet &&
       (!inst->valid_rx ||
        UTI_CompareNtp64(&inst->remote_ntp_tx, &message->transmit_ts) < 0))) {
    inst->remote_ntp_rx = message->receive_ts;
    inst->remote_ntp_tx = message->transmit_ts;
    inst->local_rx = *rx_ts;
    inst->valid_timestamps = synced_packet;

    UTI_ZeroNtp64(&inst->init_remote_ntp_tx);
    zero_local_timestamp(&inst->init_local_rx);
    inst->updated_init_timestamps = 0;
    updated_timestamps = 2;

    /* Don't use the same set of timestamps for the next sample */
    if (interleaved_packet)
      inst->prev_local_tx = inst->local_tx;
    else
      zero_local_timestamp(&inst->prev_local_tx);
  } else if (inst->mode == MODE_ACTIVE &&
             test1 && !UTI_IsZeroNtp64(&message->transmit_ts) && test5 &&
             (!inst->updated_init_timestamps ||
              UTI_CompareNtp64(&inst->init_remote_ntp_tx, &message->transmit_ts) < 0)) {
    inst->init_remote_ntp_tx = message->transmit_ts;
    inst->init_local_rx = *rx_ts;
    inst->updated_init_timestamps = 1;
    updated_timestamps = 1;
  } else {
    updated_timestamps = 0;
  }

  /* Accept at most one response per request.  The NTP specification recommends
     resetting local_ntp_tx to make the following packets fail test2 or test3,
     but that would not allow the code above to make multiple updates of the
     timestamps in symmetric mode.  Also, ignore presend responses. */
  if (inst->valid_rx) {
    test2 = test3 = 0;
    valid_packet = synced_packet = good_packet = 0;
  } else if (valid_packet) {
    if (inst->presend_done) {
      testA = 0;
      good_packet = 0;
    }
    inst->valid_rx = 1;
  }

  if ((unsigned int)local_receive.source >= sizeof (tss_chars) ||
      (unsigned int)local_transmit.source >= sizeof (tss_chars))
    assert(0);

  DEBUG_LOG("NTP packet lvm=%o stratum=%d poll=%d prec=%d root_delay=%f root_disp=%f refid=%"PRIx32" [%s]",
            message->lvm, message->stratum, message->poll, message->precision,
            pkt_root_delay, pkt_root_dispersion, pkt_refid,
            message->stratum == NTP_INVALID_STRATUM ? UTI_RefidToString(pkt_refid) : "");
  DEBUG_LOG("reference=%s origin=%s receive=%s transmit=%s",
            UTI_Ntp64ToString(&message->reference_ts),
            UTI_Ntp64ToString(&message->originate_ts),
            UTI_Ntp64ToString(&message->receive_ts),
            UTI_Ntp64ToString(&message->transmit_ts));
  DEBUG_LOG("offset=%.9f delay=%.9f dispersion=%f root_delay=%f root_dispersion=%f",
            sample.offset, sample.peer_delay, sample.peer_dispersion,
            sample.root_delay, sample.root_dispersion);
  DEBUG_LOG("remote_interval=%.9f local_interval=%.9f response_time=%.9f txs=%c rxs=%c",
            remote_interval, local_interval, response_time,
            tss_chars[local_transmit.source], tss_chars[local_receive.source]);
  DEBUG_LOG("test123=%d%d%d test567=%d%d%d testABCD=%d%d%d%d kod_rate=%d interleaved=%d"
            " presend=%d valid=%d good=%d updated=%d",
            test1, test2, test3, test5, test6, test7, testA, testB, testC, testD,
            kod_rate, interleaved_packet, inst->presend_done, valid_packet, good_packet,
            updated_timestamps);

  if (valid_packet) {
    inst->remote_poll = message->poll;
    inst->remote_stratum = message->stratum != NTP_INVALID_STRATUM ?
                           message->stratum : NTP_MAX_STRATUM;

    inst->prev_local_poll = inst->local_poll;
    inst->prev_tx_count = inst->tx_count;
    inst->tx_count = 0;

    SRC_UpdateReachability(inst->source, synced_packet);

    if (good_packet) {
      /* Adjust the polling interval, accumulate the sample, etc. */
      process_sample(inst, &sample);

      /* If we're in burst mode, check whether the burst is completed and
         revert to the previous mode */
      switch (inst->opmode) {
        case MD_BURST_WAS_ONLINE:
        case MD_BURST_WAS_OFFLINE:
          --inst->burst_good_samples_to_go;
          if (inst->burst_good_samples_to_go <= 0) {
            if (inst->opmode == MD_BURST_WAS_ONLINE)
              inst->opmode = MD_ONLINE;
            else
              take_offline(inst);
          }
          break;
        default:
          break;
      }
    } else {
      /* Slowly increase the polling interval if we can't get good packet */
      adjust_poll(inst, 0.1);
    }

    /* If in client mode, no more packets are expected to be coming from the
       server and the socket can be closed */
    close_client_socket(inst);

    /* Update the local address and interface */
    inst->local_addr.ip_addr = local_addr->ip_addr;
    inst->local_addr.if_index = local_addr->if_index;

    /* And now, requeue the timer */
    if (inst->opmode != MD_OFFLINE) {
      delay_time = get_transmit_delay(inst, 0,
                     UTI_DiffTimespecsToDouble(&inst->local_rx.ts, &inst->local_tx.ts));

      if (kod_rate) {
        LOG(LOGS_WARN, "Received KoD RATE from %s",
            UTI_IPToString(&inst->remote_addr.ip_addr));

        /* Back off for a while and stop ongoing burst */
        delay_time += 4 * UTI_Log2ToDouble(inst->local_poll);

        if (inst->opmode == MD_BURST_WAS_OFFLINE || inst->opmode == MD_BURST_WAS_ONLINE) {
          inst->burst_good_samples_to_go = 0;
        }
      }

      /* Get rid of old timeout and start a new one */
      assert(inst->tx_timeout_id);
      restart_timeout(inst, delay_time);
    }

    /* Update the NTP report */
    inst->report.remote_addr = inst->remote_addr.ip_addr;
    inst->report.local_addr = inst->local_addr.ip_addr;
    inst->report.remote_port = inst->remote_addr.port;
    inst->report.leap = pkt_leap;
    inst->report.version = pkt_version;
    inst->report.mode = NTP_LVM_TO_MODE(message->lvm);
    inst->report.stratum = message->stratum;
    inst->report.poll = message->poll;
    inst->report.precision = message->precision;
    inst->report.root_delay = pkt_root_delay;
    inst->report.root_dispersion = pkt_root_dispersion;
    inst->report.ref_id = pkt_refid;
    UTI_Ntp64ToTimespec(&message->reference_ts, &inst->report.ref_time);
    inst->report.offset = sample.offset;
    inst->report.peer_delay = sample.peer_delay;
    inst->report.peer_dispersion = sample.peer_dispersion;
    inst->report.response_time = response_time;
    inst->report.jitter_asymmetry = SST_GetJitterAsymmetry(stats);
    inst->report.tests = ((((((((test1 << 1 | test2) << 1 | test3) << 1 |
                               test5) << 1 | test6) << 1 | test7) << 1 |
                            testA) << 1 | testB) << 1 | testC) << 1 | testD;
    inst->report.interleaved = interleaved_packet;
    inst->report.authenticated = inst->auth_mode != AUTH_NONE;
    inst->report.tx_tss_char = tss_chars[local_transmit.source];
    inst->report.rx_tss_char = tss_chars[local_receive.source];

    inst->report.total_valid_count++;
  }

  /* Do measurement logging */
  if (logfileid != -1 && (log_raw_measurements || synced_packet)) {
    LOG_FileWrite(logfileid, "%s %-15s %1c %2d %1d%1d%1d %1d%1d%1d %1d%1d%1d%d  %2d %2d %4.2f %10.3e %10.3e %10.3e %10.3e %10.3e %08"PRIX32" %1d%1c %1c %1c",
            UTI_TimeToLogForm(sample.time.tv_sec),
            UTI_IPToString(&inst->remote_addr.ip_addr),
            leap_chars[pkt_leap],
            message->stratum,
            test1, test2, test3, test5, test6, test7, testA, testB, testC, testD,
            inst->local_poll, message->poll,
            inst->poll_score,
            sample.offset, sample.peer_delay, sample.peer_dispersion,
            pkt_root_delay, pkt_root_dispersion, pkt_refid,
            NTP_LVM_TO_MODE(message->lvm), interleaved_packet ? 'I' : 'B',
            tss_chars[local_transmit.source],
            tss_chars[local_receive.source]);
  }            

  return good_packet;
}

/* ================================================== */
/* From RFC 5905, the standard handling of received packets, depending
   on the mode of the packet and of the source, is :

   +------------------+---------------------------------------+
   |                  |              Packet Mode              |
   +------------------+-------+-------+-------+-------+-------+
   | Association Mode |   1   |   2   |   3   |   4   |   5   |
   +------------------+-------+-------+-------+-------+-------+
   | No Association 0 | NEWPS | DSCRD | FXMIT | MANY  | NEWBC |
   | Symm. Active   1 | PROC  | PROC  | DSCRD | DSCRD | DSCRD |
   | Symm. Passive  2 | PROC  | ERR   | DSCRD | DSCRD | DSCRD |
   | Client         3 | DSCRD | DSCRD | DSCRD | PROC  | DSCRD |
   | Server         4 | DSCRD | DSCRD | DSCRD | DSCRD | DSCRD |
   | Broadcast      5 | DSCRD | DSCRD | DSCRD | DSCRD | DSCRD |
   | Bcast Client   6 | DSCRD | DSCRD | DSCRD | DSCRD | PROC  |
   +------------------+-------+-------+-------+-------+-------+

   Association mode 0 is implemented in NCR_ProcessRxUnknown(), other modes
   in NCR_ProcessRxKnown().

   Broadcast, manycast and ephemeral symmetric passive associations are not
   supported yet.
 */

/* ================================================== */
/* This routine is called when a new packet arrives off the network,
   and it relates to a source we have an ongoing protocol exchange with */

int
NCR_ProcessRxKnown(NCR_Instance inst, NTP_Local_Address *local_addr,
                   NTP_Local_Timestamp *rx_ts, NTP_Packet *message, int length)
{
  int pkt_mode, proc_packet, proc_as_unknown;

  if (!check_packet_format(message, length))
    return 0;

  pkt_mode = NTP_LVM_TO_MODE(message->lvm);
  proc_packet = 0;
  proc_as_unknown = 0;

  /* Now, depending on the mode we decide what to do */
  switch (pkt_mode) {
    case MODE_ACTIVE:
      switch (inst->mode) {
        case MODE_ACTIVE:
          /* Ordinary symmetric peering */
          proc_packet = 1;
          break;
        case MODE_PASSIVE:
          /* In this software this case should not arise, we don't
             support unconfigured peers */
          break;
        case MODE_CLIENT:
          /* This is where we have the remote configured as a server and he has
             us configured as a peer, process as from an unknown source */
          proc_as_unknown = 1;
          break;
        default:
          /* Discard */
          break;
      }
      break;

    case MODE_PASSIVE:
      switch (inst->mode) {
        case MODE_ACTIVE:
          /* This would arise if we have the remote configured as a peer and
             he does not have us configured */
          proc_packet = 1;
          break;
        case MODE_PASSIVE:
          /* Error condition in RFC 5905 */
          break;
        default:
          /* Discard */
          break;
      }
      break;

    case MODE_CLIENT:
      /* If message is client mode, we just respond with a server mode
         packet, regardless of what we think the remote machine is
         supposed to be.  However, even though this is a configured
         peer or server, we still implement access restrictions on
         client mode operation.

         This copes with the case for an isolated network where one
         machine is set by eye and is used as the master, with the
         other machines pointed at it.  If the master goes down, we
         want to be able to reset its time at startup by relying on
         one of the secondaries to flywheel it. The behaviour coded here
         is required in the secondaries to make this possible. */

      proc_as_unknown = 1;
      break;

    case MODE_SERVER:
      switch (inst->mode) {
        case MODE_CLIENT:
          /* Standard case where he's a server and we're the client */
          proc_packet = 1;
          break;
        default:
          /* Discard */
          break;
      }
      break;

    case MODE_BROADCAST:
      /* Just ignore these */
      break;

    default:
      /* Obviously ignore */
      break;
  }

  if (proc_packet) {
    /* Check if the reply was received by the socket that sent the request */
    if (local_addr->sock_fd != inst->local_addr.sock_fd) {
      DEBUG_LOG("Packet received by wrong socket %d (expected %d)",
                local_addr->sock_fd, inst->local_addr.sock_fd);
      return 0;
    }

    /* Ignore packets from offline sources */
    if (inst->opmode == MD_OFFLINE || inst->tx_suspended) {
      DEBUG_LOG("Packet from offline source");
      return 0;
    }

    return receive_packet(inst, local_addr, rx_ts, message, length);
  } else if (proc_as_unknown) {
    NCR_ProcessRxUnknown(&inst->remote_addr, local_addr, rx_ts, message, length);
    /* It's not a reply to our request, don't return success */
    return 0;
  } else {
    DEBUG_LOG("NTP packet discarded pkt_mode=%d our_mode=%u", pkt_mode, inst->mode);
    return 0;
  }
}

/* ================================================== */
/* This routine is called when a new packet arrives off the network,
   and it relates to a source we don't know (not our server or peer) */

void
NCR_ProcessRxUnknown(NTP_Remote_Address *remote_addr, NTP_Local_Address *local_addr,
                     NTP_Local_Timestamp *rx_ts, NTP_Packet *message, int length)
{
  NTP_Mode pkt_mode, my_mode;
  NTP_int64 *local_ntp_rx, *local_ntp_tx;
  NTP_Local_Timestamp local_tx, *tx_ts;
  int pkt_version, valid_auth, log_index, interleaved, poll;
  AuthenticationMode auth_mode;
  uint32_t key_id;

  /* Ignore the packet if it wasn't received by server socket */
  if (!NIO_IsServerSocket(local_addr->sock_fd)) {
    DEBUG_LOG("NTP request packet received by client socket %d", local_addr->sock_fd);
    return;
  }

  if (!check_packet_format(message, length))
    return;

  if (!ADF_IsAllowed(access_auth_table, &remote_addr->ip_addr)) {
    DEBUG_LOG("NTP packet received from unauthorised host %s port %d",
              UTI_IPToString(&remote_addr->ip_addr),
              remote_addr->port);
    return;
  }

  pkt_mode = NTP_LVM_TO_MODE(message->lvm);
  pkt_version = NTP_LVM_TO_VERSION(message->lvm);

  switch (pkt_mode) {
    case MODE_ACTIVE:
      /* We are symmetric passive, even though we don't ever lock to him */
      my_mode = MODE_PASSIVE;
      break;
    case MODE_CLIENT:
      /* Reply with server packet */
      my_mode = MODE_SERVER;
      break;
    case MODE_UNDEFINED:
      /* Check if it is an NTPv1 client request (NTPv1 packets have a reserved
         field instead of the mode field and the actual mode is determined from
         the port numbers).  Don't ever respond with a mode 0 packet! */
      if (pkt_version == 1 && remote_addr->port != NTP_PORT) {
        my_mode = MODE_SERVER;
        break;
      }
      /* Fall through */
    default:
      /* Discard */
      DEBUG_LOG("NTP packet discarded pkt_mode=%u", pkt_mode);
      return;
  }

  log_index = CLG_LogNTPAccess(&remote_addr->ip_addr, &rx_ts->ts);

  /* Don't reply to all requests if the rate is excessive */
  if (log_index >= 0 && CLG_LimitNTPResponseRate(log_index)) {
      DEBUG_LOG("NTP packet discarded to limit response rate");
      return;
  }

  /* Check if the packet includes MAC that authenticates properly */
  valid_auth = check_packet_auth(message, length, &auth_mode, &key_id);

  /* If authentication failed, select whether and how we should respond */
  if (!valid_auth) {
    switch (auth_mode) {
      case AUTH_NONE:
        /* Reply with no MAC */
        break;
      case AUTH_MSSNTP:
        /* Ignore the failure (MS-SNTP servers don't check client MAC) */
        break;
      default:
        /* Discard packets in other modes */
        DEBUG_LOG("NTP packet discarded auth_mode=%u", auth_mode);
        return;
    }
  }

  local_ntp_rx = local_ntp_tx = NULL;
  tx_ts = NULL;
  interleaved = 0;

  /* Check if the client is using the interleaved mode.  If it is, save the
     new transmit timestamp and if the old transmit timestamp is valid, respond
     in the interleaved mode.  This means the third reply to a new client is
     the earliest one that can be interleaved.  We don't want to waste time
     on clients that are not using the interleaved mode. */
  if (log_index >= 0) {
    CLG_GetNtpTimestamps(log_index, &local_ntp_rx, &local_ntp_tx);
    interleaved = !UTI_IsZeroNtp64(local_ntp_rx) &&
                  !UTI_CompareNtp64(&message->originate_ts, local_ntp_rx) &&
                  UTI_CompareNtp64(&message->receive_ts, &message->transmit_ts);

    if (interleaved) {
      UTI_Ntp64ToTimespec(local_ntp_tx, &local_tx.ts);
      tx_ts = &local_tx;
    } else {
      UTI_ZeroNtp64(local_ntp_tx);
      local_ntp_tx = NULL;
    }
  }

  /* Suggest the client to increase its polling interval if it indicates
     the interval is shorter than the rate limiting interval */
  poll = CLG_GetNtpMinPoll();
  poll = MAX(poll, message->poll);

  /* Send a reply */
  transmit_packet(my_mode, interleaved, poll, pkt_version,
                  auth_mode, key_id, &message->receive_ts, &message->transmit_ts,
                  rx_ts, tx_ts, local_ntp_rx, NULL, remote_addr, local_addr);

  /* Save the transmit timestamp */
  if (tx_ts)
    UTI_TimespecToNtp64(&tx_ts->ts, local_ntp_tx, NULL);
}

/* ================================================== */

static void
update_tx_timestamp(NTP_Local_Timestamp *tx_ts, NTP_Local_Timestamp *new_tx_ts,
                    NTP_int64 *local_ntp_rx, NTP_int64 *local_ntp_tx, NTP_Packet *message)
{
  double delay;

  if (UTI_IsZeroTimespec(&tx_ts->ts)) {
    DEBUG_LOG("Unexpected TX update");
    return;
  }

  /* Check if this is the last packet that was sent */
  if ((local_ntp_rx && UTI_CompareNtp64(&message->receive_ts, local_ntp_rx)) ||
      (local_ntp_tx && UTI_CompareNtp64(&message->transmit_ts, local_ntp_tx))) {
    DEBUG_LOG("RX/TX timestamp mismatch");
    return;
  }

  delay = UTI_DiffTimespecsToDouble(&new_tx_ts->ts, &tx_ts->ts);

  if (delay < 0.0 || delay > MAX_TX_DELAY) {
    DEBUG_LOG("Unacceptable TX delay %.9f", delay);
    return;
  }

  *tx_ts = *new_tx_ts;

  DEBUG_LOG("Updated TX timestamp delay=%.9f", delay);
}

/* ================================================== */

void
NCR_ProcessTxKnown(NCR_Instance inst, NTP_Local_Address *local_addr,
                   NTP_Local_Timestamp *tx_ts, NTP_Packet *message, int length)
{
  NTP_Mode pkt_mode;

  if (!check_packet_format(message, length))
    return;

  pkt_mode = NTP_LVM_TO_MODE(message->lvm);

  /* Server and passive mode packets are responses to unknown sources */
  if (pkt_mode != MODE_CLIENT && pkt_mode != MODE_ACTIVE) {
    NCR_ProcessTxUnknown(&inst->remote_addr, local_addr, tx_ts, message, length);
    return;
  }

  update_tx_timestamp(&inst->local_tx, tx_ts, &inst->local_ntp_rx, &inst->local_ntp_tx,
                      message);
}

/* ================================================== */

void
NCR_ProcessTxUnknown(NTP_Remote_Address *remote_addr, NTP_Local_Address *local_addr,
                     NTP_Local_Timestamp *tx_ts, NTP_Packet *message, int length)
{
  NTP_int64 *local_ntp_rx, *local_ntp_tx;
  NTP_Local_Timestamp local_tx;
  int log_index;

  if (!check_packet_format(message, length))
    return;

  if (NTP_LVM_TO_MODE(message->lvm) == MODE_BROADCAST)
    return;

  log_index = CLG_GetClientIndex(&remote_addr->ip_addr);
  if (log_index < 0)
    return;

  if (SMT_IsEnabled() && NTP_LVM_TO_MODE(message->lvm) == MODE_SERVER)
    UTI_AddDoubleToTimespec(&tx_ts->ts, SMT_GetOffset(&tx_ts->ts), &tx_ts->ts);

  CLG_GetNtpTimestamps(log_index, &local_ntp_rx, &local_ntp_tx);

  UTI_Ntp64ToTimespec(local_ntp_tx, &local_tx.ts);
  update_tx_timestamp(&local_tx, tx_ts, local_ntp_rx, NULL, message);
  UTI_TimespecToNtp64(&local_tx.ts, local_ntp_tx, NULL);
}

/* ================================================== */

void
NCR_SlewTimes(NCR_Instance inst, struct timespec *when, double dfreq, double doffset)
{
  double delta;

  if (!UTI_IsZeroTimespec(&inst->local_rx.ts))
    UTI_AdjustTimespec(&inst->local_rx.ts, when, &inst->local_rx.ts, &delta, dfreq, doffset);
  if (!UTI_IsZeroTimespec(&inst->local_tx.ts))
    UTI_AdjustTimespec(&inst->local_tx.ts, when, &inst->local_tx.ts, &delta, dfreq, doffset);
  if (!UTI_IsZeroTimespec(&inst->prev_local_tx.ts))
    UTI_AdjustTimespec(&inst->prev_local_tx.ts, when, &inst->prev_local_tx.ts, &delta, dfreq,
                       doffset);
  if (!UTI_IsZeroTimespec(&inst->init_local_rx.ts))
    UTI_AdjustTimespec(&inst->init_local_rx.ts, when, &inst->init_local_rx.ts, &delta, dfreq,
                       doffset);

  if (inst->filter)
    SPF_SlewSamples(inst->filter, when, dfreq, doffset);
}

/* ================================================== */

void
NCR_SetConnectivity(NCR_Instance inst, SRC_Connectivity connectivity)
{
  char *s;

  s = UTI_IPToString(&inst->remote_addr.ip_addr);

  if (connectivity == SRC_MAYBE_ONLINE)
    connectivity = NIO_IsServerConnectable(&inst->remote_addr) ? SRC_ONLINE : SRC_OFFLINE;

  switch (connectivity) {
    case SRC_ONLINE:
      switch (inst->opmode) {
        case MD_ONLINE:
          /* Nothing to do */
          break;
        case MD_OFFLINE:
          LOG(LOGS_INFO, "Source %s online", s);
          inst->opmode = MD_ONLINE;
          NCR_ResetInstance(inst);
          start_initial_timeout(inst);
          break;
        case MD_BURST_WAS_ONLINE:
          /* Will revert */
          break;
        case MD_BURST_WAS_OFFLINE:
          inst->opmode = MD_BURST_WAS_ONLINE;
          LOG(LOGS_INFO, "Source %s online", s);
          break;
        default:
          assert(0);
      }
      break;
    case SRC_OFFLINE:
      switch (inst->opmode) {
        case MD_ONLINE:
          LOG(LOGS_INFO, "Source %s offline", s);
          take_offline(inst);
          break;
        case MD_OFFLINE:
          break;
        case MD_BURST_WAS_ONLINE:
          inst->opmode = MD_BURST_WAS_OFFLINE;
          LOG(LOGS_INFO, "Source %s offline", s);
          break;
        case MD_BURST_WAS_OFFLINE:
          break;
        default:
          assert(0);
      }
      break;
    default:
      assert(0);
  }
}

/* ================================================== */

void
NCR_ModifyMinpoll(NCR_Instance inst, int new_minpoll)
{
  if (new_minpoll < MIN_POLL || new_minpoll > MAX_POLL)
    return;
  inst->minpoll = new_minpoll;
  LOG(LOGS_INFO, "Source %s new minpoll %d", UTI_IPToString(&inst->remote_addr.ip_addr), new_minpoll);
  if (inst->maxpoll < inst->minpoll)
    NCR_ModifyMaxpoll(inst, inst->minpoll);
}

/* ================================================== */

void
NCR_ModifyMaxpoll(NCR_Instance inst, int new_maxpoll)
{
  if (new_maxpoll < MIN_POLL || new_maxpoll > MAX_POLL)
    return;
  inst->maxpoll = new_maxpoll;
  LOG(LOGS_INFO, "Source %s new maxpoll %d", UTI_IPToString(&inst->remote_addr.ip_addr), new_maxpoll);
  if (inst->minpoll > inst->maxpoll)
    NCR_ModifyMinpoll(inst, inst->maxpoll);
}

/* ================================================== */

void
NCR_ModifyMaxdelay(NCR_Instance inst, double new_max_delay)
{
  inst->max_delay = CLAMP(0.0, new_max_delay, MAX_MAXDELAY);
  LOG(LOGS_INFO, "Source %s new maxdelay %f",
      UTI_IPToString(&inst->remote_addr.ip_addr), inst->max_delay);
}

/* ================================================== */

void
NCR_ModifyMaxdelayratio(NCR_Instance inst, double new_max_delay_ratio)
{
  inst->max_delay_ratio = CLAMP(0.0, new_max_delay_ratio, MAX_MAXDELAYRATIO);
  LOG(LOGS_INFO, "Source %s new maxdelayratio %f",
      UTI_IPToString(&inst->remote_addr.ip_addr), inst->max_delay_ratio);
}

/* ================================================== */

void
NCR_ModifyMaxdelaydevratio(NCR_Instance inst, double new_max_delay_dev_ratio)
{
  inst->max_delay_dev_ratio = CLAMP(0.0, new_max_delay_dev_ratio, MAX_MAXDELAYDEVRATIO);
  LOG(LOGS_INFO, "Source %s new maxdelaydevratio %f",
      UTI_IPToString(&inst->remote_addr.ip_addr), inst->max_delay_dev_ratio);
}

/* ================================================== */

void
NCR_ModifyMinstratum(NCR_Instance inst, int new_min_stratum)
{
  inst->min_stratum = new_min_stratum;
  LOG(LOGS_INFO, "Source %s new minstratum %d",
      UTI_IPToString(&inst->remote_addr.ip_addr), new_min_stratum);
}

/* ================================================== */

void
NCR_ModifyPolltarget(NCR_Instance inst, int new_poll_target)
{
  inst->poll_target = new_poll_target;
  LOG(LOGS_INFO, "Source %s new polltarget %d",
      UTI_IPToString(&inst->remote_addr.ip_addr), new_poll_target);
}

/* ================================================== */

void
NCR_InitiateSampleBurst(NCR_Instance inst, int n_good_samples, int n_total_samples)
{

  if (inst->mode == MODE_CLIENT) {

    /* We want to prevent burst mode being used on symmetric active
       associations - it will play havoc with the peer's sampling
       strategy. (This obviously relies on us having the peer
       configured that way if he has us configured symmetric active -
       but there's not much else we can do.) */

    switch (inst->opmode) {
      case MD_BURST_WAS_OFFLINE:
      case MD_BURST_WAS_ONLINE:
        /* If already burst sampling, don't start again */
        break;

      case MD_ONLINE:
      case MD_OFFLINE:
        inst->opmode = inst->opmode == MD_ONLINE ?
          MD_BURST_WAS_ONLINE : MD_BURST_WAS_OFFLINE;
        inst->burst_good_samples_to_go = n_good_samples;
        inst->burst_total_samples_to_go = n_total_samples;
        start_initial_timeout(inst);
        break;
      default:
        assert(0);
        break;
    }
  }

}

/* ================================================== */

void
NCR_ReportSource(NCR_Instance inst, RPT_SourceReport *report, struct timespec *now)
{
  report->poll = get_transmit_poll(inst);

  switch (inst->mode) {
    case MODE_CLIENT:
      report->mode = RPT_NTP_CLIENT;
      break;
    case MODE_ACTIVE:
      report->mode = RPT_NTP_PEER;
      break;
    default:
      assert(0);
  }
}

/* ================================================== */

void
NCR_GetNTPReport(NCR_Instance inst, RPT_NTPReport *report)
{
  *report = inst->report;
}

/* ================================================== */

int
NCR_AddAccessRestriction(IPAddr *ip_addr, int subnet_bits, int allow, int all)
 {
  ADF_Status status;

  if (allow) {
    if (all) {
      status = ADF_AllowAll(access_auth_table, ip_addr, subnet_bits);
    } else {
      status = ADF_Allow(access_auth_table, ip_addr, subnet_bits);
    }
  } else {
    if (all) {
      status = ADF_DenyAll(access_auth_table, ip_addr, subnet_bits);
    } else {
      status = ADF_Deny(access_auth_table, ip_addr, subnet_bits);
    }
  }

  if (status != ADF_SUCCESS)
    return 0;

  /* Keep server sockets open only when an address allowed */
  if (allow) {
    NTP_Remote_Address remote_addr;

    if (server_sock_fd4 == INVALID_SOCK_FD &&
        ADF_IsAnyAllowed(access_auth_table, IPADDR_INET4)) {
      remote_addr.ip_addr.family = IPADDR_INET4;
      server_sock_fd4 = NIO_OpenServerSocket(&remote_addr);
    }
    if (server_sock_fd6 == INVALID_SOCK_FD &&
        ADF_IsAnyAllowed(access_auth_table, IPADDR_INET6)) {
      remote_addr.ip_addr.family = IPADDR_INET6;
      server_sock_fd6 = NIO_OpenServerSocket(&remote_addr);
    }
  } else {
    if (server_sock_fd4 != INVALID_SOCK_FD &&
        !ADF_IsAnyAllowed(access_auth_table, IPADDR_INET4)) {
      NIO_CloseServerSocket(server_sock_fd4);
      server_sock_fd4 = INVALID_SOCK_FD;
    }
    if (server_sock_fd6 != INVALID_SOCK_FD &&
        !ADF_IsAnyAllowed(access_auth_table, IPADDR_INET6)) {
      NIO_CloseServerSocket(server_sock_fd6);
      server_sock_fd6 = INVALID_SOCK_FD;
    }
  }

  return 1;
}

/* ================================================== */

int
NCR_CheckAccessRestriction(IPAddr *ip_addr)
{
  return ADF_IsAllowed(access_auth_table, ip_addr);
}

/* ================================================== */

void
NCR_IncrementActivityCounters(NCR_Instance inst, int *online, int *offline,
                              int *burst_online, int *burst_offline)
{
  switch (inst->opmode) {
    case MD_BURST_WAS_OFFLINE:
      ++*burst_offline;
      break;
    case MD_BURST_WAS_ONLINE:
      ++*burst_online;
      break;
    case MD_ONLINE:
      ++*online;
      break;
    case MD_OFFLINE:
      ++*offline;
      break;
    default:
      assert(0);
      break;
  }
}

/* ================================================== */

NTP_Remote_Address *
NCR_GetRemoteAddress(NCR_Instance inst) 
{
  return &inst->remote_addr;
}

/* ================================================== */

uint32_t
NCR_GetLocalRefid(NCR_Instance inst)
{
  return UTI_IPToRefid(&inst->local_addr.ip_addr);
}

/* ================================================== */

int NCR_IsSyncPeer(NCR_Instance inst)
{
  return SRC_IsSyncPeer(inst->source);
}

/* ================================================== */

static void
broadcast_timeout(void *arg)
{
  BroadcastDestination *destination;
  NTP_int64 orig_ts;
  NTP_Local_Timestamp recv_ts;
  int poll;

  destination = ARR_GetElement(broadcasts, (long)arg);
  poll = log(destination->interval) / log(2.0) + 0.5;

  UTI_ZeroNtp64(&orig_ts);
  zero_local_timestamp(&recv_ts);

  transmit_packet(MODE_BROADCAST, 0, poll, NTP_VERSION, 0, 0, &orig_ts, &orig_ts, &recv_ts,
                  NULL, NULL, NULL, &destination->addr, &destination->local_addr);

  /* Requeue timeout.  We don't care if interval drifts gradually. */
  SCH_AddTimeoutInClass(destination->interval, get_separation(poll), SAMPLING_RANDOMNESS,
                        SCH_NtpBroadcastClass, broadcast_timeout, arg);
}

/* ================================================== */

void
NCR_AddBroadcastDestination(IPAddr *addr, unsigned short port, int interval)
{
  BroadcastDestination *destination;

  destination = (BroadcastDestination *)ARR_GetNewElement(broadcasts);

  destination->addr.ip_addr = *addr;
  destination->addr.port = port;
  destination->local_addr.ip_addr.family = IPADDR_UNSPEC;
  destination->local_addr.if_index = INVALID_IF_INDEX;
  destination->local_addr.sock_fd = NIO_OpenServerSocket(&destination->addr);
  destination->interval = CLAMP(1, interval, 1 << MAX_POLL);

  SCH_AddTimeoutInClass(destination->interval, MAX_SAMPLING_SEPARATION, SAMPLING_RANDOMNESS,
                        SCH_NtpBroadcastClass, broadcast_timeout,
                        (void *)(long)(ARR_GetSize(broadcasts) - 1));
}
