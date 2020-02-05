/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * Copyright (C) Miroslav Lichvar  2011-2012, 2014, 2016
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

  Functions which manage the pool of NTP sources that we are currently
  a client of or peering with.

  */

#include "config.h"

#include "sysincl.h"

#include "array.h"
#include "ntp_sources.h"
#include "ntp_core.h"
#include "util.h"
#include "logging.h"
#include "local.h"
#include "memory.h"
#include "nameserv_async.h"
#include "privops.h"
#include "sched.h"

/* ================================================== */

/* Record type private to this file, used to store information about
   particular sources */
typedef struct {
  NTP_Remote_Address *remote_addr; /* The address of this source, non-NULL
                                      means this slot in table is in use */
  NCR_Instance data;            /* Data for the protocol engine for this source */
  char *name;                   /* Name of the source, may be NULL */
  int pool;                     /* Number of the pool from which was this source
                                   added or INVALID_POOL */
  int tentative;                /* Flag indicating there was no valid response
                                   received from the source yet */
} SourceRecord;

/* Hash table of SourceRecord, its size is a power of two and it's never
   more than half full */
static ARR_Instance records;

/* Number of sources in the hash table */
static int n_sources;

/* Flag indicating new sources will be started automatically when added */
static int auto_start_sources = 0;

/* Source with unknown address (which may be resolved later) */
struct UnresolvedSource {
  char *name;
  int port;
  int random_order;
  int replacement;
  union {
    struct {
      NTP_Source_Type type;
      SourceParameters params;
      int pool;
      int max_new_sources;
    } new_source;
    NTP_Remote_Address replace_source;
  };
  struct UnresolvedSource *next;
};

#define RESOLVE_INTERVAL_UNIT 7
#define MIN_RESOLVE_INTERVAL 2
#define MAX_RESOLVE_INTERVAL 9
#define MIN_REPLACEMENT_INTERVAL 8

static struct UnresolvedSource *unresolved_sources = NULL;
static int resolving_interval = 0;
static SCH_TimeoutID resolving_id;
static struct UnresolvedSource *resolving_source = NULL;
static NSR_SourceResolvingEndHandler resolving_end_handler = NULL;

#define MAX_POOL_SOURCES 16
#define INVALID_POOL (-1)

/* Pool of sources with the same name */
struct SourcePool {
  /* Number of sources added from this pool (ignoring tentative sources) */
  int sources;
  /* Maximum number of sources */
  int max_sources;
};

/* Array of SourcePool */
static ARR_Instance pools;

/* ================================================== */
/* Forward prototypes */

static void resolve_sources(void *arg);
static void rehash_records(void);
static void clean_source_record(SourceRecord *record);

static void
slew_sources(struct timespec *raw,
             struct timespec *cooked,
             double dfreq,
             double doffset,
             LCL_ChangeType change_type,
             void *anything);

/* ================================================== */

/* Flag indicating whether module is initialised */
static int initialised = 0;

/* ================================================== */

static SourceRecord *
get_record(unsigned index)
{
  return (SourceRecord *)ARR_GetElement(records, index);
}

/* ================================================== */

void
NSR_Initialise(void)
{
  n_sources = 0;
  initialised = 1;

  records = ARR_CreateInstance(sizeof (SourceRecord));
  rehash_records();

  pools = ARR_CreateInstance(sizeof (struct SourcePool));

  LCL_AddParameterChangeHandler(slew_sources, NULL);
}

/* ================================================== */

void
NSR_Finalise(void)
{
  SourceRecord *record;
  struct UnresolvedSource *us;
  unsigned int i;

  ARR_DestroyInstance(pools);

  for (i = 0; i < ARR_GetSize(records); i++) {
    record = get_record(i);
    if (record->remote_addr)
      clean_source_record(record);
  }

  ARR_DestroyInstance(records);

  while (unresolved_sources) {
    us = unresolved_sources;
    unresolved_sources = us->next;
    Free(us->name);
    Free(us);
  }

  initialised = 0;
}

/* ================================================== */
/* Return slot number and whether the IP address was matched or not.
   found = 0 => Neither IP nor port matched, empty slot returned
   found = 1 => Only IP matched, port doesn't match
   found = 2 => Both IP and port matched.

   It is assumed that there can only ever be one record for a
   particular IP address.  (If a different port comes up, it probably
   means someone is running ntpdate -d or something).  Thus, if we
   match the IP address we stop the search regardless of whether the
   port number matches.

  */

static void
find_slot(NTP_Remote_Address *remote_addr, int *slot, int *found)
{
  SourceRecord *record;
  uint32_t hash;
  unsigned int i, size;
  unsigned short port;

  size = ARR_GetSize(records);

  *slot = 0;
  *found = 0;
  
  if (remote_addr->ip_addr.family != IPADDR_INET4 &&
      remote_addr->ip_addr.family != IPADDR_INET6)
    return;

  hash = UTI_IPToHash(&remote_addr->ip_addr);
  port = remote_addr->port;

  for (i = 0; i < size / 2; i++) {
    /* Use quadratic probing */
    *slot = (hash + (i + i * i) / 2) % size;
    record = get_record(*slot);

    if (!record->remote_addr)
      break;

    if (!UTI_CompareIPs(&record->remote_addr->ip_addr,
                        &remote_addr->ip_addr, NULL)) {
      *found = record->remote_addr->port == port ? 2 : 1;
      return;
    }
  }
}

/* ================================================== */
/* Check if hash table of given size is sufficient to contain sources */

static int
check_hashtable_size(unsigned int sources, unsigned int size)
{
  return sources * 2 <= size;
}

/* ================================================== */

static void
rehash_records(void)
{
  SourceRecord *temp_records;
  unsigned int i, old_size, new_size;
  int slot, found;

  old_size = ARR_GetSize(records);

  temp_records = MallocArray(SourceRecord, old_size);
  memcpy(temp_records, ARR_GetElements(records), old_size * sizeof (SourceRecord));

  /* The size of the hash table is always a power of two */
  for (new_size = 1; !check_hashtable_size(n_sources, new_size); new_size *= 2)
    ;

  ARR_SetSize(records, new_size);

  for (i = 0; i < new_size; i++)
    get_record(i)->remote_addr = NULL;

  for (i = 0; i < old_size; i++) {
    if (!temp_records[i].remote_addr)
      continue;

    find_slot(temp_records[i].remote_addr, &slot, &found);
    assert(!found);

    *get_record(slot) = temp_records[i];
  }

  Free(temp_records);
}

/* ================================================== */

/* Procedure to add a new source */
static NSR_Status
add_source(NTP_Remote_Address *remote_addr, char *name, NTP_Source_Type type, SourceParameters *params, int pool)
{
  SourceRecord *record;
  int slot, found;

  assert(initialised);

  /* Find empty bin & check that we don't have the address already */
  find_slot(remote_addr, &slot, &found);
  if (found) {
    return NSR_AlreadyInUse;
  } else {
    if (remote_addr->ip_addr.family != IPADDR_INET4 &&
               remote_addr->ip_addr.family != IPADDR_INET6) {
      return NSR_InvalidAF;
    } else {
      n_sources++;

      if (!check_hashtable_size(n_sources, ARR_GetSize(records))) {
        rehash_records();
        find_slot(remote_addr, &slot, &found);
      }

      assert(!found);
      record = get_record(slot);
      record->data = NCR_GetInstance(remote_addr, type, params);
      record->remote_addr = NCR_GetRemoteAddress(record->data);
      record->name = name ? Strdup(name) : NULL;
      record->pool = pool;
      record->tentative = 1;

      if (auto_start_sources)
        NCR_StartInstance(record->data);

      return NSR_Success;
    }
  }
}

/* ================================================== */

static NSR_Status
replace_source(NTP_Remote_Address *old_addr, NTP_Remote_Address *new_addr)
{
  int slot1, slot2, found;
  SourceRecord *record;
  struct SourcePool *pool;

  find_slot(old_addr, &slot1, &found);
  if (!found)
    return NSR_NoSuchSource;

  find_slot(new_addr, &slot2, &found);
  if (found)
    return NSR_AlreadyInUse;

  record = get_record(slot1);
  NCR_ChangeRemoteAddress(record->data, new_addr);
  record->remote_addr = NCR_GetRemoteAddress(record->data);

  if (!record->tentative) {
    record->tentative = 1;

    if (record->pool != INVALID_POOL) {
      pool = ARR_GetElement(pools, record->pool);
      pool->sources--;
    }
  }

  /* The hash table must be rebuilt for the new address */
  rehash_records();

  LOG(LOGS_INFO, "Source %s replaced with %s",
      UTI_IPToString(&old_addr->ip_addr),
      UTI_IPToString(&new_addr->ip_addr));

  return NSR_Success;
}

/* ================================================== */

static void
process_resolved_name(struct UnresolvedSource *us, IPAddr *ip_addrs, int n_addrs)
{
  NTP_Remote_Address address;
  int i, added;
  unsigned short first = 0;

  if (us->random_order)
    UTI_GetRandomBytes(&first, sizeof (first));

  for (i = added = 0; i < n_addrs; i++) {
    address.ip_addr = ip_addrs[((unsigned int)i + first) % n_addrs];
    address.port = us->port;

    DEBUG_LOG("(%d) %s", i + 1, UTI_IPToString(&address.ip_addr));

    if (us->replacement) {
      if (replace_source(&us->replace_source, &address) != NSR_AlreadyInUse)
        break;
    } else {
      if (add_source(&address, us->name, us->new_source.type, &us->new_source.params,
                     us->new_source.pool) == NSR_Success)
        added++;

      if (added >= us->new_source.max_new_sources)
        break;
    }
  }
}

/* ================================================== */

static void
name_resolve_handler(DNS_Status status, int n_addrs, IPAddr *ip_addrs, void *anything)
{
  struct UnresolvedSource *us, **i, *next;

  us = (struct UnresolvedSource *)anything;

  assert(us == resolving_source);

  DEBUG_LOG("%s resolved to %d addrs", us->name, n_addrs);

  switch (status) {
    case DNS_TryAgain:
      break;
    case DNS_Success:
      process_resolved_name(us, ip_addrs, n_addrs);
      break;
    case DNS_Failure:
      LOG(LOGS_WARN, "Invalid host %s", us->name);
      break;
    default:
      assert(0);
  }

  next = us->next;

  /* Remove the source from the list on success or failure, replacements
     are removed on any status */
  if (us->replacement || status != DNS_TryAgain) {
    for (i = &unresolved_sources; *i; i = &(*i)->next) {
      if (*i == us) {
        *i = us->next;
        Free(us->name);
        Free(us);
        break;
      }
    }
  }

  resolving_source = next;

  if (next) {
    /* Continue with the next source in the list */
    DEBUG_LOG("resolving %s", next->name);
    DNS_Name2IPAddressAsync(next->name, name_resolve_handler, next);
  } else {
    /* This was the last source in the list. If some sources couldn't
       be resolved, try again in exponentially increasing interval. */
    if (unresolved_sources) {
      if (resolving_interval < MIN_RESOLVE_INTERVAL)
        resolving_interval = MIN_RESOLVE_INTERVAL;
      else if (resolving_interval < MAX_RESOLVE_INTERVAL)
        resolving_interval++;
      resolving_id = SCH_AddTimeoutByDelay(RESOLVE_INTERVAL_UNIT *
          (1 << resolving_interval), resolve_sources, NULL);
    } else {
      resolving_interval = 0;
    }

    /* This round of resolving is done */
    if (resolving_end_handler)
      (resolving_end_handler)();
  }
}

/* ================================================== */

static void
resolve_sources(void *arg)
{
  struct UnresolvedSource *us;

  assert(!resolving_source);

  PRV_ReloadDNS();

  /* Start with the first source in the list, name_resolve_handler
     will iterate over the rest */
  us = unresolved_sources;

  resolving_source = us;
  DEBUG_LOG("resolving %s", us->name);
  DNS_Name2IPAddressAsync(us->name, name_resolve_handler, us);
}

/* ================================================== */

static void
append_unresolved_source(struct UnresolvedSource *us)
{
  struct UnresolvedSource **i;

  for (i = &unresolved_sources; *i; i = &(*i)->next)
    ;
  *i = us;
  us->next = NULL;
}

/* ================================================== */

NSR_Status
NSR_AddSource(NTP_Remote_Address *remote_addr, NTP_Source_Type type, SourceParameters *params)
{
  return add_source(remote_addr, NULL, type, params, INVALID_POOL);
}

/* ================================================== */

void
NSR_AddSourceByName(char *name, int port, int pool, NTP_Source_Type type, SourceParameters *params)
{
  struct UnresolvedSource *us;
  struct SourcePool *sp;
  NTP_Remote_Address remote_addr;

  /* If the name is an IP address, don't bother with full resolving now
     or later when trying to replace the source */
  if (UTI_StringToIP(name, &remote_addr.ip_addr)) {
    remote_addr.port = port;
    NSR_AddSource(&remote_addr, type, params);
    return;
  }

  us = MallocNew(struct UnresolvedSource);
  us->name = Strdup(name);
  us->port = port;
  us->random_order = 0;
  us->replacement = 0;
  us->new_source.type = type;
  us->new_source.params = *params;

  if (!pool) {
    us->new_source.pool = INVALID_POOL;
    us->new_source.max_new_sources = 1;
  } else {
    sp = (struct SourcePool *)ARR_GetNewElement(pools);
    sp->sources = 0;
    sp->max_sources = params->max_sources;
    us->new_source.pool = ARR_GetSize(pools) - 1;
    us->new_source.max_new_sources = MAX_POOL_SOURCES;
  }

  append_unresolved_source(us);
}

/* ================================================== */

void
NSR_SetSourceResolvingEndHandler(NSR_SourceResolvingEndHandler handler)
{
  resolving_end_handler = handler;
}

/* ================================================== */

void
NSR_ResolveSources(void)
{
  /* Try to resolve unresolved sources now */
  if (unresolved_sources) {
    /* Make sure no resolving is currently running */
    if (!resolving_source) {
      if (resolving_interval) {
        SCH_RemoveTimeout(resolving_id);
        resolving_interval--;
      }
      resolve_sources(NULL);
    }
  } else {
    /* No unresolved sources, we are done */
    if (resolving_end_handler)
      (resolving_end_handler)();
  }
}

/* ================================================== */

void NSR_StartSources(void)
{
  unsigned int i;

  for (i = 0; i < ARR_GetSize(records); i++) {
    if (!get_record(i)->remote_addr)
      continue;
    NCR_StartInstance(get_record(i)->data);
  }
}

/* ================================================== */

void NSR_AutoStartSources(void)
{
  auto_start_sources = 1;
}

/* ================================================== */

static void
clean_source_record(SourceRecord *record)
{
  assert(record->remote_addr);
  record->remote_addr = NULL;
  NCR_DestroyInstance(record->data);
  if (record->name)
    Free(record->name);

  n_sources--;
}

/* ================================================== */

/* Procedure to remove a source.  We don't bother whether the port
   address is matched - we're only interested in removing a record for
   the right IP address.  Thus the caller can specify the port number
   as zero if it wishes. */
NSR_Status
NSR_RemoveSource(NTP_Remote_Address *remote_addr)
{
  int slot, found;

  assert(initialised);

  find_slot(remote_addr, &slot, &found);
  if (!found) {
    return NSR_NoSuchSource;
  }

  clean_source_record(get_record(slot));

  /* Rehash the table to make sure there are no broken probe sequences.
     This is costly, but it's not expected to happen frequently. */

  rehash_records();

  return NSR_Success;
}

/* ================================================== */

void
NSR_RemoveAllSources(void)
{
  SourceRecord *record;
  unsigned int i;

  for (i = 0; i < ARR_GetSize(records); i++) {
    record = get_record(i);
    if (!record->remote_addr)
      continue;
    clean_source_record(record);
  }

  rehash_records();
}

/* ================================================== */

static void
resolve_source_replacement(SourceRecord *record)
{
  struct UnresolvedSource *us;

  DEBUG_LOG("trying to replace %s", UTI_IPToString(&record->remote_addr->ip_addr));

  us = MallocNew(struct UnresolvedSource);
  us->name = Strdup(record->name);
  us->port = record->remote_addr->port;
  /* If there never was a valid reply from this source (e.g. it was a bad
     replacement), ignore the order of addresses from the resolver to not get
     stuck to a pair of addresses if the order doesn't change, or a group of
     IPv4/IPv6 addresses if the resolver prefers inaccessible IP family */
  us->random_order = record->tentative;
  us->replacement = 1;
  us->replace_source = *record->remote_addr;

  append_unresolved_source(us);
  NSR_ResolveSources();
}

/* ================================================== */

void
NSR_HandleBadSource(IPAddr *address)
{
  static struct timespec last_replacement;
  struct timespec now;
  NTP_Remote_Address remote_addr;
  SourceRecord *record;
  int slot, found;
  double diff;

  remote_addr.ip_addr = *address;
  remote_addr.port = 0;

  find_slot(&remote_addr, &slot, &found);
  if (!found)
    return;

  record = get_record(slot);

  /* Only sources with a name can be replaced */
  if (!record->name)
    return;

  /* Don't resolve names too frequently */
  SCH_GetLastEventTime(NULL, NULL, &now);
  diff = UTI_DiffTimespecsToDouble(&now, &last_replacement);
  if (fabs(diff) < RESOLVE_INTERVAL_UNIT * (1 << MIN_REPLACEMENT_INTERVAL)) {
    DEBUG_LOG("replacement postponed");
    return;
  }
  last_replacement = now;

  resolve_source_replacement(record);
}

/* ================================================== */

void
NSR_RefreshAddresses(void)
{
  SourceRecord *record;
  unsigned int i;

  for (i = 0; i < ARR_GetSize(records); i++) {
    record = get_record(i);
    if (!record->remote_addr || !record->name)
      continue;

    resolve_source_replacement(record);
  }
}

/* ================================================== */

static void remove_tentative_pool_sources(int pool)
{
  SourceRecord *record;
  unsigned int i, removed;

  for (i = removed = 0; i < ARR_GetSize(records); i++) {
    record = get_record(i);

    if (!record->remote_addr || record->pool != pool || !record->tentative)
      continue;

    DEBUG_LOG("removing tentative source %s",
              UTI_IPToString(&record->remote_addr->ip_addr));

    clean_source_record(record);
    removed++;
  }

  if (removed)
    rehash_records();
}

/* ================================================== */

uint32_t
NSR_GetLocalRefid(IPAddr *address)
{
  NTP_Remote_Address remote_addr;
  int slot, found;

  remote_addr.ip_addr = *address;
  remote_addr.port = 0;

  find_slot(&remote_addr, &slot, &found);
  if (!found)
    return 0;

  return NCR_GetLocalRefid(get_record(slot)->data);
}

/* ================================================== */

/* This routine is called by ntp_io when a new packet arrives off the network,
   possibly with an authentication tail */
void
NSR_ProcessRx(NTP_Remote_Address *remote_addr, NTP_Local_Address *local_addr,
              NTP_Local_Timestamp *rx_ts, NTP_Packet *message, int length)
{
  SourceRecord *record;
  struct SourcePool *pool;
  int slot, found;

  assert(initialised);

  find_slot(remote_addr, &slot, &found);
  if (found == 2) { /* Must match IP address AND port number */
    record = get_record(slot);

    if (!NCR_ProcessRxKnown(record->data, local_addr, rx_ts, message, length))
      return;

    if (record->tentative) {
      /* This was the first good reply from the source */
      record->tentative = 0;

      if (record->pool != INVALID_POOL) {
        pool = ARR_GetElement(pools, record->pool);
        pool->sources++;

        DEBUG_LOG("pool %s has %d confirmed sources", record->name, pool->sources);

        /* If the number of sources from the pool reached the configured
           maximum, remove the remaining tentative sources */
        if (pool->sources >= pool->max_sources)
          remove_tentative_pool_sources(record->pool);
      }
    }
  } else {
    NCR_ProcessRxUnknown(remote_addr, local_addr, rx_ts, message, length);
  }
}

/* ================================================== */

void
NSR_ProcessTx(NTP_Remote_Address *remote_addr, NTP_Local_Address *local_addr,
              NTP_Local_Timestamp *tx_ts, NTP_Packet *message, int length)
{
  SourceRecord *record;
  int slot, found;

  find_slot(remote_addr, &slot, &found);

  if (found == 2) { /* Must match IP address AND port number */
    record = get_record(slot);
    NCR_ProcessTxKnown(record->data, local_addr, tx_ts, message, length);
  } else {
    NCR_ProcessTxUnknown(remote_addr, local_addr, tx_ts, message, length);
  }
}

/* ================================================== */

static void
slew_sources(struct timespec *raw,
             struct timespec *cooked,
             double dfreq,
             double doffset,
             LCL_ChangeType change_type,
             void *anything)
{
  SourceRecord *record;
  unsigned int i;

  for (i = 0; i < ARR_GetSize(records); i++) {
    record = get_record(i);
    if (record->remote_addr) {
      if (change_type == LCL_ChangeUnknownStep) {
        NCR_ResetInstance(record->data);
        NCR_ResetPoll(record->data);
      } else {
        NCR_SlewTimes(record->data, cooked, dfreq, doffset);
      }
    }
  }
}

/* ================================================== */

int
NSR_SetConnectivity(IPAddr *mask, IPAddr *address, SRC_Connectivity connectivity)
{
  SourceRecord *record, *syncpeer;
  unsigned int i, any;

  if (connectivity != SRC_OFFLINE)
    NSR_ResolveSources();

  any = 0;
  syncpeer = NULL;
  for (i = 0; i < ARR_GetSize(records); i++) {
    record = get_record(i);
    if (record->remote_addr) {
      if (address->family == IPADDR_UNSPEC ||
          !UTI_CompareIPs(&record->remote_addr->ip_addr, address, mask)) {
        any = 1;
        if (NCR_IsSyncPeer(record->data)) {
          syncpeer = record;
          continue;
        }
        NCR_SetConnectivity(record->data, connectivity);
      }
    }
  }

  /* Set the sync peer last to avoid unnecessary reference switching */
  if (syncpeer)
    NCR_SetConnectivity(syncpeer->data, connectivity);

  if (address->family == IPADDR_UNSPEC) {
    struct UnresolvedSource *us;

    for (us = unresolved_sources; us; us = us->next) {
      if (us->replacement)
        continue;
      any = 1;
      us->new_source.params.connectivity = connectivity;
    }
  }

  return any;
}

/* ================================================== */

int
NSR_ModifyMinpoll(IPAddr *address, int new_minpoll)
{
  int slot, found;
  NTP_Remote_Address addr;
  addr.ip_addr = *address;
  addr.port = 0;

  find_slot(&addr, &slot, &found);
  if (found == 0) {
    return 0;
  } else {
    NCR_ModifyMinpoll(get_record(slot)->data, new_minpoll);
    return 1;
  }
}

/* ================================================== */

int
NSR_ModifyMaxpoll(IPAddr *address, int new_maxpoll)
{
  int slot, found;
  NTP_Remote_Address addr;
  addr.ip_addr = *address;
  addr.port = 0;

  find_slot(&addr, &slot, &found);
  if (found == 0) {
    return 0;
  } else {
    NCR_ModifyMaxpoll(get_record(slot)->data, new_maxpoll);
    return 1;
  }
}

/* ================================================== */

int
NSR_ModifyMaxdelay(IPAddr *address, double new_max_delay)
{
  int slot, found;
  NTP_Remote_Address addr;
  addr.ip_addr = *address;
  addr.port = 0;

  find_slot(&addr, &slot, &found);
  if (found == 0) {
    return 0;
  } else {
    NCR_ModifyMaxdelay(get_record(slot)->data, new_max_delay);
    return 1;
  }
}

/* ================================================== */

int
NSR_ModifyMaxdelayratio(IPAddr *address, double new_max_delay_ratio)
{
  int slot, found;
  NTP_Remote_Address addr;
  addr.ip_addr = *address;
  addr.port = 0;

  find_slot(&addr, &slot, &found);
  if (found == 0) {
    return 0;
  } else {
    NCR_ModifyMaxdelayratio(get_record(slot)->data, new_max_delay_ratio);
    return 1;
  }
}

/* ================================================== */

int
NSR_ModifyMaxdelaydevratio(IPAddr *address, double new_max_delay_dev_ratio)
{
  int slot, found;
  NTP_Remote_Address addr;
  addr.ip_addr = *address;
  addr.port = 0;

  find_slot(&addr, &slot, &found);
  if (found == 0) {
    return 0;
  } else {
    NCR_ModifyMaxdelaydevratio(get_record(slot)->data, new_max_delay_dev_ratio);
    return 1;
  }
}

/* ================================================== */

int
NSR_ModifyMinstratum(IPAddr *address, int new_min_stratum)
{
  int slot, found;
  NTP_Remote_Address addr;
  addr.ip_addr = *address;
  addr.port = 0;

  find_slot(&addr, &slot, &found);
  if (found == 0) {
    return 0;
  } else {
    NCR_ModifyMinstratum(get_record(slot)->data, new_min_stratum);
    return 1;
  }
}

/* ================================================== */

int
NSR_ModifyPolltarget(IPAddr *address, int new_poll_target)
{
  int slot, found;
  NTP_Remote_Address addr;
  addr.ip_addr = *address;
  addr.port = 0;

  find_slot(&addr, &slot, &found);
  if (found == 0) {
    return 0;
  } else {
    NCR_ModifyPolltarget(get_record(slot)->data, new_poll_target);
    return 1;
  }
}

/* ================================================== */

int
NSR_InitiateSampleBurst(int n_good_samples, int n_total_samples,
                        IPAddr *mask, IPAddr *address)
{
  SourceRecord *record;
  unsigned int i;
  int any;

  any = 0;
  for (i = 0; i < ARR_GetSize(records); i++) {
    record = get_record(i);
    if (record->remote_addr) {
      if (address->family == IPADDR_UNSPEC ||
          !UTI_CompareIPs(&record->remote_addr->ip_addr, address, mask)) {
        any = 1;
        NCR_InitiateSampleBurst(record->data, n_good_samples, n_total_samples);
      }
    }
  }

  return any;

}

/* ================================================== */
/* The ip address is assumed to be completed on input, that is how we
   identify the source record. */

void
NSR_ReportSource(RPT_SourceReport *report, struct timespec *now)
{
  NTP_Remote_Address rem_addr;
  int slot, found;

  rem_addr.ip_addr = report->ip_addr;
  rem_addr.port = 0;
  find_slot(&rem_addr, &slot, &found);
  if (found) {
    NCR_ReportSource(get_record(slot)->data, report, now);
  } else {
    report->poll = 0;
    report->latest_meas_ago = 0;
  }
}

/* ================================================== */
/* The ip address is assumed to be completed on input, that is how we
   identify the source record. */

int
NSR_GetNTPReport(RPT_NTPReport *report)
{
  NTP_Remote_Address rem_addr;
  int slot, found;

  rem_addr.ip_addr = report->remote_addr;
  rem_addr.port = 0;
  find_slot(&rem_addr, &slot, &found);
  if (!found)
    return 0;

  NCR_GetNTPReport(get_record(slot)->data, report);
  return 1;
}

/* ================================================== */

void
NSR_GetActivityReport(RPT_ActivityReport *report)
{
  SourceRecord *record;
  unsigned int i;
  struct UnresolvedSource *us;

  report->online = 0;
  report->offline = 0;
  report->burst_online = 0;
  report->burst_offline = 0;

  for (i = 0; i < ARR_GetSize(records); i++) {
    record = get_record(i);
    if (record->remote_addr) {
      NCR_IncrementActivityCounters(record->data, &report->online, &report->offline,
                                    &report->burst_online, &report->burst_offline);
    }
  }

  report->unresolved = 0;

  for (us = unresolved_sources; us; us = us->next) {
    report->unresolved++;
  }
}


/* ================================================== */

