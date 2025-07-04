/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * Copyright (C) Miroslav Lichvar  2011-2012, 2014, 2016, 2020-2024
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
#include "conf.h"
#include "ntp_sources.h"
#include "ntp_core.h"
#include "ntp_io.h"
#include "util.h"
#include "logging.h"
#include "local.h"
#include "memory.h"
#include "nameserv_async.h"
#include "privops.h"
#include "sched.h"

/* ================================================== */

/* Maximum number of sources */
#define MAX_SOURCES 65536

/* Record type private to this file, used to store information about
   particular sources */
typedef struct {
  NTP_Remote_Address *remote_addr; /* The address of this source, non-NULL
                                      means this slot in table is in use
                                      (an IPADDR_ID address means the address
                                      is not resolved yet) */
  NCR_Instance data;            /* Data for the protocol engine for this source */
  char *name;                   /* Name of the source as it was specified
                                   (may be an IP address) */
  IPAddr resolved_addr;         /* Address resolved from the name, which can be
                                   different from remote_addr (e.g. NTS-KE) */
  int family;                   /* IP family of acceptable resolved addresses
                                   (IPADDR_UNSPEC if any) */
  int pool_id;                  /* ID of the pool from which was this source
                                   added or INVALID_POOL */
  int tentative;                /* Flag indicating there was no valid response
                                   received from the source yet */
  uint32_t conf_id;             /* Configuration ID, which can be shared with
                                   different sources in case of a pool */
  double last_resolving;        /* Time of last name resolving (monotonic) */
} SourceRecord;

/* Hash table of SourceRecord, its size is a power of two and it's never
   more than half full */
static ARR_Instance records;

/* Number of sources in the hash table */
static int n_sources;

/* Flag indicating new sources will be started automatically when added */
static int auto_start_sources = 0;

/* Flag indicating a record is currently being modified */
static int record_lock;

/* Last assigned address ID */
static uint32_t last_address_id = 0;

/* Last assigned configuration ID */
static uint32_t last_conf_id = 0;

/* Source scheduled for name resolving (first resolving or replacement) */
struct UnresolvedSource {
  /* Current address of the source (IPADDR_ID is used for a single source
     with unknown address and IPADDR_UNSPEC for a pool of sources) */
  NTP_Remote_Address address;
  /* ID of the pool if not a single source */
  int pool_id;
  /* Name to be resolved */
  char *name;
  /* Address family to filter resolved addresses */
  int family;
  /* Flag indicating addresses should be used in a random order */
  int random_order;
  /* Flag indicating current address should be replaced only if it is
     no longer returned by the resolver */
  int refreshment;
  /* Next unresolved source in the list */
  struct UnresolvedSource *next;
};

#define RESOLVE_INTERVAL_UNIT 7
#define MIN_RESOLVE_INTERVAL 2
#define MAX_RESOLVE_INTERVAL 9
#define MAX_REPLACEMENT_INTERVAL 9

static struct UnresolvedSource *unresolved_sources = NULL;
static int resolving_interval = 0;
static int resolving_restart = 0;
static SCH_TimeoutID resolving_id;
static struct UnresolvedSource *resolving_source = NULL;
static NSR_SourceResolvingEndHandler resolving_end_handler = NULL;

#define MAX_POOL_SOURCES 16
#define INVALID_POOL (-1)

/* Pool of sources with the same name */
struct SourcePool {
  /* Number of all sources from the pool */
  int sources;
  /* Number of sources with unresolved address */
  int unresolved_sources;
  /* Number of non-tentative sources */
  int confirmed_sources;
  /* Maximum number of confirmed sources */
  int max_sources;
};

/* Array of SourcePool (indexed by their ID) */
static ARR_Instance pools;

/* Requested update of a source's address */
struct AddressUpdate {
  NTP_Remote_Address old_address;
  NTP_Remote_Address new_address;
};

/* Update saved when record_lock is true */
static struct AddressUpdate saved_address_update;

/* ================================================== */
/* Forward prototypes */

static void resolve_sources(void);
static void rehash_records(void);
static void handle_saved_address_update(void);
static void clean_source_record(SourceRecord *record);
static void remove_pool_sources(int pool_id, int tentative, int unresolved);
static void remove_unresolved_source(struct UnresolvedSource *us);

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

static struct SourcePool *
get_pool(unsigned index)
{
  return (struct SourcePool *)ARR_GetElement(pools, index);
}

/* ================================================== */

void
NSR_Initialise(void)
{
  n_sources = 0;
  resolving_id = 0;
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
  NSR_RemoveAllSources();

  LCL_RemoveParameterChangeHandler(slew_sources, NULL);

  ARR_DestroyInstance(records);
  ARR_DestroyInstance(pools);

  SCH_RemoveTimeout(resolving_id);

  /* Leave the unresolved sources allocated if the async resolver is running
     to avoid reading the name from freed memory.  The handler will not be
     called as the scheduler should no longer be running at this point. */
  if (!resolving_source) {
    while (unresolved_sources)
      remove_unresolved_source(unresolved_sources);
  }

  initialised = 0;
}

/* ================================================== */
/* Find a slot matching an IP address.  It is assumed that there can
   only ever be one record for a particular IP address. */

static int
find_slot(IPAddr *ip_addr, int *slot)
{
  SourceRecord *record;
  uint32_t hash;
  unsigned int i, size;

  size = ARR_GetSize(records);

  *slot = 0;
  
  switch (ip_addr->family) {
    case IPADDR_INET4:
    case IPADDR_INET6:
    case IPADDR_ID:
      break;
    default:
      return 0;
  }

  hash = UTI_IPToHash(ip_addr);

  for (i = 0; i < size / 2; i++) {
    /* Use quadratic probing */
    *slot = (hash + (i + i * i) / 2) % size;
    record = get_record(*slot);

    if (!record->remote_addr)
      break;

    if (UTI_CompareIPs(&record->remote_addr->ip_addr, ip_addr, NULL) == 0)
      return 1;
  }

  return 0;
}

/* ================================================== */
/* Find a slot matching an IP address and port. The function returns:
   0 => IP not matched, empty slot returned if a valid address was provided
   1 => Only IP matched, port doesn't match
   2 => Both IP and port matched. */

static int
find_slot2(NTP_Remote_Address *remote_addr, int *slot)
{
  if (!find_slot(&remote_addr->ip_addr, slot))
    return 0;

  return get_record(*slot)->remote_addr->port == remote_addr->port ? 2 : 1;
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
  int slot;

  assert(!record_lock);

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

    if (find_slot2(temp_records[i].remote_addr, &slot) != 0)
      assert(0);

    *get_record(slot) = temp_records[i];
  }

  Free(temp_records);
}

/* ================================================== */

static void
log_source(SourceRecord *record, int addition, int once_per_pool)
{
  int pool, log_addr;
  char *ip_str;

  if (once_per_pool && record->pool_id != INVALID_POOL) {
    if (get_pool(record->pool_id)->sources > 1)
      return;
    pool = 1;
    log_addr = 0;
  } else {
    ip_str = UTI_IPToString(&record->remote_addr->ip_addr);
    pool = 0;
    log_addr = strcmp(record->name, ip_str) != 0;
  }

  LOG(LOG_GetContextSeverity(LOGC_Command | LOGC_SourceFile), "%s %s %s%s%s%s",
      addition ? "Added" : "Removed", pool ? "pool" : "source",
      log_addr ? ip_str : record->name,
      log_addr ? " (" : "", log_addr ? record->name : "", log_addr ? ")" : "");
}

/* ================================================== */

/* Procedure to add a new source */
static NSR_Status
add_source(NTP_Remote_Address *remote_addr, char *name, int family, NTP_Source_Type type,
           SourceParameters *params, int pool_id, uint32_t conf_id)
{
  SourceRecord *record;
  int slot;

  assert(initialised);

  /* Find empty bin & check that we don't have the address already */
  if (find_slot2(remote_addr, &slot) != 0) {
    return NSR_AlreadyInUse;
  } else if (!name && !UTI_IsIPReal(&remote_addr->ip_addr)) {
    /* Name is required for non-real addresses */
    return NSR_InvalidName;
  } else if (n_sources >= MAX_SOURCES) {
    return NSR_TooManySources;
  } else {
    if (remote_addr->ip_addr.family != IPADDR_INET4 &&
        remote_addr->ip_addr.family != IPADDR_INET6 &&
        remote_addr->ip_addr.family != IPADDR_ID) {
      return NSR_InvalidAF;
    } else {
      n_sources++;

      if (!check_hashtable_size(n_sources, ARR_GetSize(records))) {
        rehash_records();
        if (find_slot2(remote_addr, &slot) != 0)
          assert(0);
      }

      assert(!record_lock);
      record_lock = 1;

      record = get_record(slot);
      record->name = Strdup(name ? name : UTI_IPToString(&remote_addr->ip_addr));
      record->data = NCR_CreateInstance(remote_addr, type, params, record->name);
      record->remote_addr = NCR_GetRemoteAddress(record->data);
      record->resolved_addr = remote_addr->ip_addr;
      record->family = family;
      record->pool_id = pool_id;
      record->tentative = 1;
      record->conf_id = conf_id;
      record->last_resolving = SCH_GetLastEventMonoTime();

      record_lock = 0;

      if (record->pool_id != INVALID_POOL) {
        get_pool(record->pool_id)->sources++;
        if (!UTI_IsIPReal(&remote_addr->ip_addr))
          get_pool(record->pool_id)->unresolved_sources++;
      }

      if (auto_start_sources && UTI_IsIPReal(&remote_addr->ip_addr))
        NCR_StartInstance(record->data);

      log_source(record, 1, 1);

      /* The new instance is allowed to change its address immediately */
      handle_saved_address_update();

      return NSR_Success;
    }
  }
}

/* ================================================== */

static NSR_Status
change_source_address(NTP_Remote_Address *old_addr, NTP_Remote_Address *new_addr,
                      int replacement)
{
  int slot1, slot2, found;
  SourceRecord *record;
  LOG_Severity severity;
  char *name;

  found = find_slot2(old_addr, &slot1);
  if (found != 2)
    return NSR_NoSuchSource;

  /* Make sure there is no other source using the new address (with the same
     or different port), but allow a source to have its port changed */
  found = find_slot2(new_addr, &slot2);
  if (found == 2 || (found != 0 && slot1 != slot2))
    return NSR_AlreadyInUse;

  assert(!record_lock);
  record_lock = 1;

  record = get_record(slot1);
  NCR_ChangeRemoteAddress(record->data, new_addr, !replacement);
  if (replacement)
    record->resolved_addr = new_addr->ip_addr;

  BRIEF_ASSERT(record->remote_addr == NCR_GetRemoteAddress(record->data) &&
               UTI_CompareIPs(&record->remote_addr->ip_addr, &new_addr->ip_addr, NULL) == 0);

  if (!UTI_IsIPReal(&old_addr->ip_addr) && UTI_IsIPReal(&new_addr->ip_addr)) {
    if (auto_start_sources)
      NCR_StartInstance(record->data);
    if (record->pool_id != INVALID_POOL)
      get_pool(record->pool_id)->unresolved_sources--;
  }

  if (!record->tentative) {
    record->tentative = 1;

    if (record->pool_id != INVALID_POOL)
      get_pool(record->pool_id)->confirmed_sources--;
  }

  record_lock = 0;

  name = record->name;
  severity = UTI_IsIPReal(&old_addr->ip_addr) ? LOGS_INFO : LOGS_DEBUG;

  if (found == 0) {
    /* The hash table must be rebuilt for the changed address */
    rehash_records();

    LOG(severity, "Source %s %s %s (%s)", UTI_IPToString(&old_addr->ip_addr),
        replacement ? "replaced with" : "changed to",
        UTI_IPToString(&new_addr->ip_addr), name);
  } else {
    LOG(severity, "Source %s (%s) changed port to %d",
        UTI_IPToString(&new_addr->ip_addr), name, new_addr->port);
  }

  return NSR_Success;
}

/* ================================================== */

static void
handle_saved_address_update(void)
{
  if (!UTI_IsIPReal(&saved_address_update.old_address.ip_addr))
    return;

  if (change_source_address(&saved_address_update.old_address,
                            &saved_address_update.new_address, 0) != NSR_Success)
    /* This is expected to happen only if the old address is wrong */
    LOG(LOGS_ERR, "Could not change %s to %s",
        UTI_IPSockAddrToString(&saved_address_update.old_address),
        UTI_IPSockAddrToString(&saved_address_update.new_address));

  saved_address_update.old_address.ip_addr.family = IPADDR_UNSPEC;
}

/* ================================================== */

static int
replace_source_connectable(NTP_Remote_Address *old_addr, NTP_Remote_Address *new_addr)
{
  if (!NIO_IsServerConnectable(new_addr)) {
    DEBUG_LOG("%s not connectable", UTI_IPToString(&new_addr->ip_addr));
    return 0;
  }

  if (change_source_address(old_addr, new_addr, 1) == NSR_AlreadyInUse)
    return 0;

  handle_saved_address_update();

  return 1;
}

/* ================================================== */

static void
process_resolved_name(struct UnresolvedSource *us, IPAddr *ip_addrs, int n_addrs)
{
  NTP_Remote_Address old_addr, new_addr;
  SourceRecord *record;
  unsigned short first = 0;
  int i, j, slot;

  /* Keep using the current address if it is being refreshed and it is
     still included in the resolved addresses */
  if (us->refreshment) {
    assert(us->pool_id == INVALID_POOL);

    for (i = 0; i < n_addrs; i++) {
      if (find_slot2(&us->address, &slot) == 2 &&
          UTI_CompareIPs(&get_record(slot)->resolved_addr, &ip_addrs[i], NULL) == 0) {
        DEBUG_LOG("%s still fresh", UTI_IPToString(&us->address.ip_addr));
        return;
      }
    }
  }

  if (us->random_order)
    UTI_GetRandomBytes(&first, sizeof (first));

  for (i = 0; i < n_addrs; i++) {
    new_addr.ip_addr = ip_addrs[((unsigned int)i + first) % n_addrs];

    DEBUG_LOG("(%d) %s", i + 1, UTI_IPToString(&new_addr.ip_addr));

    /* Skip addresses not from the requested family */
    if (us->family != IPADDR_UNSPEC && us->family != new_addr.ip_addr.family)
      continue;

    if (us->pool_id != INVALID_POOL) {
      /* In the pool resolving mode, try to replace a source from
         the pool which does not have a real address yet */
      for (j = 0; j < ARR_GetSize(records); j++) {
        record = get_record(j);
        if (!record->remote_addr || record->pool_id != us->pool_id ||
            UTI_IsIPReal(&record->remote_addr->ip_addr))
          continue;
        old_addr = *record->remote_addr;
        new_addr.port = old_addr.port;
        if (replace_source_connectable(&old_addr, &new_addr))
          ;
        break;
      }
    } else {
      new_addr.port = us->address.port;
      if (replace_source_connectable(&us->address, &new_addr))
        break;
    }
  }
}

/* ================================================== */

static int
is_resolved(struct UnresolvedSource *us)
{
  int slot;

  if (us->pool_id != INVALID_POOL) {
    return get_pool(us->pool_id)->unresolved_sources <= 0;
  } else {
    /* If the address is no longer present, it was removed or replaced
       (i.e. resolved) */
    return find_slot2(&us->address, &slot) == 0;
  }
}

/* ================================================== */

static void
resolve_sources_timeout(void *arg)
{
  resolving_id = 0;
  resolve_sources();
}

/* ================================================== */

static void
name_resolve_handler(DNS_Status status, int n_addrs, IPAddr *ip_addrs, void *anything)
{
  struct UnresolvedSource *us, *next;

  us = (struct UnresolvedSource *)anything;

  assert(us == resolving_source);
  assert(resolving_id == 0);

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

  /* Don't repeat the resolving if it (permanently) failed, it was a
     replacement of a real address, a refreshment, or all addresses are
     already resolved */
  if (status == DNS_Failure || UTI_IsIPReal(&us->address.ip_addr) ||
      us->refreshment || is_resolved(us))
    remove_unresolved_source(us);

  /* If a restart was requested and this was the last source in the list,
     start with the first source again (if there still is one) */
  if (!next && resolving_restart) {
    DEBUG_LOG("Restarting");
    next = unresolved_sources;
    resolving_restart = 0;
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
      resolving_interval = CLAMP(MIN_RESOLVE_INTERVAL, resolving_interval + 1,
                                 MAX_RESOLVE_INTERVAL);
      resolving_id = SCH_AddTimeoutByDelay(RESOLVE_INTERVAL_UNIT * (1 << resolving_interval),
                                           resolve_sources_timeout, NULL);
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
resolve_sources(void)
{
  struct UnresolvedSource *us, *next, *i;

  assert(!resolving_source);

  /* Remove sources that don't need to be resolved anymore */
  for (i = unresolved_sources; i; i = next) {
    next = i->next;
    if (is_resolved(i))
      remove_unresolved_source(i);
  }

  if (!unresolved_sources)
    return;

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
  int n;

  for (i = &unresolved_sources, n = 0; *i; i = &(*i)->next, n++)
    ;
  *i = us;
  us->next = NULL;

  DEBUG_LOG("Added unresolved source #%d pool_id=%d random=%d refresh=%d",
            n + 1, us->pool_id, us->random_order, us->refreshment);
}

/* ================================================== */

static void
remove_unresolved_source(struct UnresolvedSource *us)
{
  struct UnresolvedSource **i;

  for (i = &unresolved_sources; *i; i = &(*i)->next) {
    if (*i == us) {
      *i = us->next;
      Free(us->name);
      Free(us);
      break;
    }
  }
}

/* ================================================== */

static int get_unused_pool_id(void)
{
  struct UnresolvedSource *us;
  int i;

  for (i = 0; i < ARR_GetSize(pools); i++) {
    if (get_pool(i)->sources > 0)
      continue;

    /* Make sure there is no name waiting to be resolved using this pool */
    for (us = unresolved_sources; us; us = us->next) {
      if (us->pool_id == i)
        break;
    }
    if (us)
      continue;

    return i;
  }

  return INVALID_POOL;
}

/* ================================================== */

static uint32_t
get_next_conf_id(uint32_t *conf_id)
{
  SourceRecord *record;
  unsigned int i;

again:
  last_conf_id++;

  /* Make sure the ID is not already used (after 32-bit wraparound) */
  for (i = 0; i < ARR_GetSize(records); i++) {
    record = get_record(i);
    if (record->remote_addr && record->conf_id == last_conf_id)
      goto again;
  }

  if (conf_id)
    *conf_id = last_conf_id;

  return last_conf_id;
}

/* ================================================== */

NSR_Status
NSR_AddSource(NTP_Remote_Address *remote_addr, NTP_Source_Type type,
              SourceParameters *params, uint32_t *conf_id)
{
  return add_source(remote_addr, NULL, IPADDR_UNSPEC, type, params, INVALID_POOL,
                    get_next_conf_id(conf_id));
}

/* ================================================== */

NSR_Status
NSR_AddSourceByName(char *name, int family, int port, int pool, NTP_Source_Type type,
                    SourceParameters *params, uint32_t *conf_id)
{
  struct UnresolvedSource *us;
  struct SourcePool *sp;
  NTP_Remote_Address remote_addr;
  int i, new_sources, pool_id;
  uint32_t cid;

  /* If the name is an IP address, add the source with the address directly */
  if (UTI_StringToIP(name, &remote_addr.ip_addr)) {
    remote_addr.port = port;
    if (family != IPADDR_UNSPEC && family != remote_addr.ip_addr.family)
      return NSR_InvalidAF;
    return add_source(&remote_addr, name, IPADDR_UNSPEC, type, params, INVALID_POOL,
                      get_next_conf_id(conf_id));
  }

  /* Make sure the name is at least printable and has no spaces */
  for (i = 0; name[i] != '\0'; i++) {
    if (!isgraph((unsigned char)name[i]))
      return NSR_InvalidName;
  }

  us = MallocNew(struct UnresolvedSource);
  us->name = Strdup(name);
  us->family = family;
  us->random_order = 0;
  us->refreshment = 0;

  remote_addr.ip_addr.family = IPADDR_ID;
  remote_addr.ip_addr.addr.id = ++last_address_id;
  remote_addr.port = port;

  if (!pool) {
    us->pool_id = INVALID_POOL;
    us->address = remote_addr;
    new_sources = 1;
  } else {
    pool_id = get_unused_pool_id();
    if (pool_id != INVALID_POOL) {
      sp = get_pool(pool_id);
    } else {
      sp = ARR_GetNewElement(pools);
      pool_id = ARR_GetSize(pools) - 1;
    }

    sp->sources = 0;
    sp->unresolved_sources = 0;
    sp->confirmed_sources = 0;
    sp->max_sources = CLAMP(1, params->max_sources, MAX_POOL_SOURCES);
    us->pool_id = pool_id;
    us->address.ip_addr.family = IPADDR_UNSPEC;
    new_sources = MIN(2 * sp->max_sources, MAX_POOL_SOURCES);
  }

  append_unresolved_source(us);

  cid = get_next_conf_id(conf_id);

  for (i = 0; i < new_sources; i++) {
    if (i > 0)
      remote_addr.ip_addr.addr.id = ++last_address_id;
    if (add_source(&remote_addr, name, family, type, params, us->pool_id, cid) != NSR_Success)
      return NSR_TooManySources;
  }

  return NSR_UnresolvedName;
}

/* ================================================== */

const char *
NSR_StatusToString(NSR_Status status)
{
  switch (status) {
    case NSR_Success:
      return "Success";
    case NSR_NoSuchSource:
      return "No such source";
    case NSR_AlreadyInUse:
      return "Already in use";
    case NSR_TooManySources:
      return "Too many sources";
    case NSR_InvalidAF:
      return "Invalid address";
    case NSR_InvalidName:
      return "Invalid name";
    case NSR_UnresolvedName:
      return "Unresolved name";
    default:
      return "?";
  }
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
    /* Allow only one resolving to be running at a time */
    if (!resolving_source) {
      if (resolving_id != 0) {
        SCH_RemoveTimeout(resolving_id);
        resolving_id = 0;
        resolving_interval--;
      }
      resolve_sources();
    } else {
      /* Try again as soon as the current resolving ends */
      resolving_restart = 1;
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
  NTP_Remote_Address *addr;
  unsigned int i;

  for (i = 0; i < ARR_GetSize(records); i++) {
    addr = get_record(i)->remote_addr;
    if (!addr || !UTI_IsIPReal(&addr->ip_addr))
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

  if (record->pool_id != INVALID_POOL) {
    struct SourcePool *pool = get_pool(record->pool_id);

    pool->sources--;
    if (!UTI_IsIPReal(&record->remote_addr->ip_addr))
      pool->unresolved_sources--;
    if (!record->tentative)
      pool->confirmed_sources--;
    if (pool->max_sources > pool->sources)
      pool->max_sources = pool->sources;
  }

  record->remote_addr = NULL;
  NCR_DestroyInstance(record->data);
  Free(record->name);

  n_sources--;
}

/* ================================================== */

/* Procedure to remove a source.  We don't bother whether the port
   address is matched - we're only interested in removing a record for
   the right IP address. */
NSR_Status
NSR_RemoveSource(IPAddr *address)
{
  int slot;

  assert(initialised);

  if (find_slot(address, &slot) == 0)
    return NSR_NoSuchSource;

  log_source(get_record(slot), 0, 0);
  clean_source_record(get_record(slot));

  /* Rehash the table to make sure there are no broken probe sequences.
     This is costly, but it's not expected to happen frequently. */

  rehash_records();

  return NSR_Success;
}

/* ================================================== */

void
NSR_RemoveSourcesById(uint32_t conf_id)
{
  SourceRecord *record;
  unsigned int i;

  for (i = 0; i < ARR_GetSize(records); i++) {
    record = get_record(i);
    if (!record->remote_addr || record->conf_id != conf_id)
      continue;
    log_source(record, 0, 1);
    clean_source_record(record);
  }

  rehash_records();
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
resolve_source_replacement(SourceRecord *record, int refreshment)
{
  struct UnresolvedSource *us;

  DEBUG_LOG("%s %s (%s)", refreshment ? "refreshing" : "trying to replace",
            UTI_IPToString(&record->remote_addr->ip_addr), record->name);

  record->last_resolving = SCH_GetLastEventMonoTime();

  us = MallocNew(struct UnresolvedSource);
  us->name = Strdup(record->name);
  us->family = record->family;
  /* Ignore the order of addresses from the resolver to not get
     stuck with a pair of unreachable or otherwise unusable servers
     (e.g. falsetickers) in case the order doesn't change, or a group
     of servers if they are ordered by IP family */
  us->random_order = 1;
  us->refreshment = refreshment;
  us->pool_id = INVALID_POOL;
  us->address = *record->remote_addr;

  append_unresolved_source(us);

  /* Don't restart resolving round if already running */
  if (!resolving_source)
    NSR_ResolveSources();
}

/* ================================================== */

void
NSR_HandleBadSource(IPAddr *address)
{
  static double next_replacement = 0.0;
  SourceRecord *record;
  IPAddr ip_addr;
  uint32_t rnd;
  double now;
  int slot;

  if (!find_slot(address, &slot))
    return;

  record = get_record(slot);

  /* Don't try to replace a source specified by an IP address unless the
     address changed since the source was added (e.g. by NTS-KE) */
  if (UTI_StringToIP(record->name, &ip_addr) &&
      UTI_CompareIPs(&record->remote_addr->ip_addr, &ip_addr, NULL) == 0)
    return;

  /* Don't resolve names too frequently */
  now = SCH_GetLastEventMonoTime();
  if (now < next_replacement) {
    DEBUG_LOG("replacement postponed");
    return;
  }

  UTI_GetRandomBytes(&rnd, sizeof (rnd));
  next_replacement = now + ((double)rnd / (uint32_t)-1) *
                     (RESOLVE_INTERVAL_UNIT * (1 << MAX_REPLACEMENT_INTERVAL));

  resolve_source_replacement(record, 0);
}

/* ================================================== */

static void
maybe_refresh_source(void)
{
  static double last_refreshment = 0.0;
  SourceRecord *record, *oldest_record;
  int i, min_interval;
  double now;

  min_interval = CNF_GetRefresh();

  now = SCH_GetLastEventMonoTime();
  if (min_interval <= 0 || now < last_refreshment + min_interval)
    return;

  last_refreshment = now;

  for (i = 0, oldest_record = NULL; i < ARR_GetSize(records); i++) {
    record = get_record(i);
    if (!record->remote_addr || UTI_IsStringIP(record->name))
      continue;

    if (!oldest_record || oldest_record->last_resolving > record->last_resolving)
      oldest_record = record;
  }

  if (!oldest_record)
    return;

  /* Check if the name wasn't already resolved in the last interval */
  if (now < oldest_record->last_resolving + min_interval) {
    last_refreshment = oldest_record->last_resolving;
    return;
  }

  resolve_source_replacement(oldest_record, 1);
}

/* ================================================== */

void
NSR_RefreshAddresses(void)
{
  SourceRecord *record;
  unsigned int i;

  for (i = 0; i < ARR_GetSize(records); i++) {
    record = get_record(i);
    if (!record->remote_addr)
      continue;

    resolve_source_replacement(record, 1);
  }
}

/* ================================================== */

NSR_Status
NSR_UpdateSourceNtpAddress(NTP_Remote_Address *old_addr, NTP_Remote_Address *new_addr)
{
  int slot;

  if (!UTI_IsIPReal(&old_addr->ip_addr) || !UTI_IsIPReal(&new_addr->ip_addr))
    return NSR_InvalidAF;

  if (UTI_CompareIPs(&old_addr->ip_addr, &new_addr->ip_addr, NULL) != 0 &&
      find_slot(&new_addr->ip_addr, &slot))
    return NSR_AlreadyInUse;

  /* If a record is being modified (e.g. by change_source_address(), or the
     source is just being created), postpone the change to avoid corruption */

  if (!record_lock)
    return change_source_address(old_addr, new_addr, 0);

  if (UTI_IsIPReal(&saved_address_update.old_address.ip_addr))
    return NSR_TooManySources;

  saved_address_update.old_address = *old_addr;
  saved_address_update.new_address = *new_addr;

  return NSR_Success;
}

/* ================================================== */

static void remove_pool_sources(int pool_id, int tentative, int unresolved)
{
  SourceRecord *record;
  unsigned int i, removed;

  for (i = removed = 0; i < ARR_GetSize(records); i++) {
    record = get_record(i);

    if (!record->remote_addr || record->pool_id != pool_id)
      continue;

    if ((tentative && !record->tentative) ||
        (unresolved && UTI_IsIPReal(&record->remote_addr->ip_addr)))
      continue;

    DEBUG_LOG("removing %ssource %s", tentative ? "tentative " : "",
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
  int slot;

  if (!find_slot(address, &slot))
    return 0;

  return NCR_GetLocalRefid(get_record(slot)->data);
}

/* ================================================== */

char *
NSR_GetName(IPAddr *address)
{
  int slot;

  if (!find_slot(address, &slot))
    return NULL;

  return get_record(slot)->name;
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
  int slot;

  assert(initialised);

  /* Avoid unnecessary lookup if the packet cannot be a response from our
     source.  Otherwise, it must match both IP address and port number. */
  if (NTP_LVM_TO_MODE(message->lvm) != MODE_CLIENT &&
      find_slot2(remote_addr, &slot) == 2) {
    record = get_record(slot);

    if (!NCR_ProcessRxKnown(record->data, local_addr, rx_ts, message, length))
      return;

    if (record->tentative) {
      /* This was the first good reply from the source */
      record->tentative = 0;

      if (record->pool_id != INVALID_POOL) {
        pool = get_pool(record->pool_id);
        pool->confirmed_sources++;

        DEBUG_LOG("pool %s has %d confirmed sources", record->name, pool->confirmed_sources);

        /* If the number of sources from the pool reached the configured
           maximum, remove the remaining tentative sources */
        if (pool->confirmed_sources >= pool->max_sources)
          remove_pool_sources(record->pool_id, 1, 0);
      }
    }

    maybe_refresh_source();
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
  int slot;

  /* Avoid unnecessary lookup if the packet cannot be a request to our
     source.  Otherwise, it must match both IP address and port number. */
  if (NTP_LVM_TO_MODE(message->lvm) != MODE_SERVER &&
      find_slot2(remote_addr, &slot) == 2) {
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
      /* Ignore SRC_MAYBE_ONLINE connectivity change for unspecified unresolved
         sources as they would always end up in the offline state */
      if ((address->family == IPADDR_UNSPEC &&
           (connectivity != SRC_MAYBE_ONLINE || UTI_IsIPReal(&record->remote_addr->ip_addr))) ||
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

  return any;
}

/* ================================================== */

int
NSR_ModifyMinpoll(IPAddr *address, int new_minpoll)
{
  int slot;

  if (!find_slot(address, &slot))
    return 0;

  NCR_ModifyMinpoll(get_record(slot)->data, new_minpoll);
  return 1;
}

/* ================================================== */

int
NSR_ModifyMaxpoll(IPAddr *address, int new_maxpoll)
{
  int slot;

  if (!find_slot(address, &slot))
    return 0;

  NCR_ModifyMaxpoll(get_record(slot)->data, new_maxpoll);
  return 1;
}

/* ================================================== */

int
NSR_ModifyMaxdelay(IPAddr *address, double new_max_delay)
{
  int slot;

  if (!find_slot(address, &slot))
    return 0;

  NCR_ModifyMaxdelay(get_record(slot)->data, new_max_delay);
  return 1;
}

/* ================================================== */

int
NSR_ModifyMaxdelayratio(IPAddr *address, double new_max_delay_ratio)
{
  int slot;

  if (!find_slot(address, &slot))
    return 0;

  NCR_ModifyMaxdelayratio(get_record(slot)->data, new_max_delay_ratio);
  return 1;
}

/* ================================================== */

int
NSR_ModifyMaxdelaydevratio(IPAddr *address, double new_max_delay_dev_ratio)
{
  int slot;

  if (!find_slot(address, &slot))
    return 0;

  NCR_ModifyMaxdelaydevratio(get_record(slot)->data, new_max_delay_dev_ratio);
  return 1;
}

/* ================================================== */

int
NSR_ModifyMinstratum(IPAddr *address, int new_min_stratum)
{
  int slot;

  if (!find_slot(address, &slot))
    return 0;

  NCR_ModifyMinstratum(get_record(slot)->data, new_min_stratum);
  return 1;
}

/* ================================================== */

int
NSR_ModifyOffset(IPAddr *address, double new_offset)
{
  int slot;

  if (!find_slot(address, &slot))
    return 0;

  NCR_ModifyOffset(get_record(slot)->data, new_offset);
  return 1;
}

/* ================================================== */

int
NSR_ModifyPolltarget(IPAddr *address, int new_poll_target)
{
  int slot;

  if (!find_slot(address, &slot))
    return 0;

  NCR_ModifyPolltarget(get_record(slot)->data, new_poll_target);
  return 1;
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
  int slot;

  if (find_slot(&report->ip_addr, &slot)) {
    NCR_ReportSource(get_record(slot)->data, report, now);
  } else {
    report->poll = 0;
    report->latest_meas_ago = 0;
  }
}

/* ================================================== */

int
NSR_GetAuthReport(IPAddr *address, RPT_AuthReport *report)
{
  int slot;

  if (!find_slot(address, &slot))
    return 0;

  NCR_GetAuthReport(get_record(slot)->data, report);
  return 1;
}

/* ================================================== */
/* The ip address is assumed to be completed on input, that is how we
   identify the source record. */

int
NSR_GetNTPReport(RPT_NTPReport *report)
{
  int slot;

  if (!find_slot(&report->remote_addr, &slot))
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

  report->online = 0;
  report->offline = 0;
  report->burst_online = 0;
  report->burst_offline = 0;
  report->unresolved = 0;

  for (i = 0; i < ARR_GetSize(records); i++) {
    record = get_record(i);
    if (!record->remote_addr)
      continue;

    if (!UTI_IsIPReal(&record->remote_addr->ip_addr)) {
      report->unresolved++;
    } else {
      NCR_IncrementActivityCounters(record->data, &report->online, &report->offline,
                                    &report->burst_online, &report->burst_offline);
    }
  }
}

/* ================================================== */

void
NSR_DumpAuthData(void)
{
  SourceRecord *record;
  int i;

  for (i = 0; i < ARR_GetSize(records); i++) {
    record = get_record(i);
    if (!record->remote_addr)
      continue;
    NCR_DumpAuthData(record->data);
  }
}
