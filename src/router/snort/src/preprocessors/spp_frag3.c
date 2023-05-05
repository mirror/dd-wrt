/* $Id$ */

/**
 * @file    spp_frag3.c
 * @author  Martin Roesch <roesch@sourcefire.com>
 * @date    Thu Sep 30 14:12:37 EDT 2004
 *
 * @brief   Frag3: IP defragmentation preprocessor for Snort.
 */

/*
 ** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 ** Copyright (C) 2004-2013 Sourcefire, Inc.
 **
 ** This program is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License Version 2 as
 ** published by the Free Software Foundation.  You may not use, modify or
 ** distribute this program under any other version of the GNU General
 ** Public License.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program; if not, write to the Free Software
 ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/*
 * Notes:
 * Frag3 sports the following improvements over frag2:
 *  - Target-based IP defragmentation, harder to evade
 *  - 8 Anomaly detection event types
 *  - Two separate memory management strategies to tailor
 *    performance for specific environments
 *  - Up to 250% faster than frag2.
 *
 *  The mechanism for processing frags is based on the Linux IP stack
 *  implementation of IP defragmentation with proper amounts of paranoia
 *  and an IDS perspective applied.  Some of this code was derived from
 *  frag2 originally, but it's basically unrecognizeable if you compare
 *  it to frag2 IMO.
 *
 *  I switched from using the UBI libs to using sfxhash and linked lists for
 *  fragment management because I suspected that the management code was
 *  the cause of performance issues that we were observing at Sourcefire
 *  in certain customer situations.  Splay trees are cool and really hard
 *  to screw with from an attack perspective, but they also incur a lot
 *  of overhead for managing the tree and lose the order of the fragments in
 *  the FragTracker's fraglist, so I dropped them.  Originally the
 *  frag3 code was just supposed to migrate away from the splay tree system
 *  that I was using in frag2, but I figured since I was doing the work to
 *  pull out the splay trees I may as well solve some of the other problems
 *  we were seeing.
 *
 *  Initial performance testing that I've done shows that frag3 can be as much
 *  as 250% faster than frag2, but we still need to do more testing and
 *  optimization, we may be able to squeeze out some more performance.
 *
 *  Frag3 is also capable of performing "Target-based" IP defragmentation.
 *  What this means practically is that frag3 can model the IP stack of a
 *  target on the network to avoid Ptacek-Newsham evasions of the IDS through
 *  sensor/target desynchronization.  In terms of implentation, this is
 *  reflected by passing a "context" into the defragmentation engine that has
 *  a specific configuration for a specific target type.  Windows can put
 *  fragments back together differently than Linux/BSD/etc, so we model that
 *  inside frag3 so we can't be evaded.
 *
 *  Configuration of frag3 is pretty straight forward, there's a global config
 *  that contains data about how the hash tables will be structured, what type
 *  of memory management to use and whether or not to generate alerts, then
 *  specific target-contexts are setup and bound to IP address sets.  Check
 *  the README file for specifics!
 */

/*  I N C L U D E S  ************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>
#include <rpc/types.h>
#include <errno.h>

#include "spp_frag3.h"
#include "snort_bounds.h"
#include "generators.h"
#include "log.h"
#include "detect.h"
#include "decode.h"
#include "encode.h"
#include "event.h"
#include "util.h"
#include "snort_debug.h"
#include "plugbase.h"
#include "parser.h"
#include "mstring.h"
#include "checksum.h"
#include "perf.h"
#include "event_queue.h"
#include "timersub.h"
#include "fpcreate.h"

#include "sfutil/sflsq.h"
#include "sfutil/sfxhash.h"

#include "snort.h"
#include "memory_stats.h"
#include "profiler.h"

#include "active.h"
#include "session_api.h"
#include "spp_normalize.h"
#include "reload.h"

#ifdef REG_TEST
#include "reg_test.h"
#endif

#ifdef TARGET_BASED
#include "sftarget_hostentry.h"
#include "sftarget_protocol_reference.h"
#endif
#include "sfPolicy.h"

extern OptTreeNode *otn_tmp;

/*  D E F I N E S  **************************************************/
#define PP_FRAG3_PRIORITY PRIORITY_CORE + PP_CORE_ORDER_FRAG3

/* flags for the FragTracker->frag_flags field */
#define FRAG_GOT_FIRST      0x00000001
#define FRAG_GOT_LAST       0x00000002
#define FRAG_REBUILT        0x00000004
#define FRAG_BAD            0x00000008
#define FRAG_NO_BSD_VULN    0x00000010
#define FRAG_DROP_FRAGMENTS 0x00000020

/* default frag timeout, 90-120 might be better values, can we do
 * target-based quanta?  */
#define FRAG_PRUNE_QUANTA   60

/* default 4MB memcap */
#define FRAG_MEMCAP   4194304

/* min acceptable ttl (should be 1?) */
#define FRAG3_MIN_TTL        1

/* target-based defragmentation policy enums */
#define FRAG_POLICY_FIRST       1
#define FRAG_POLICY_LINUX       2
#define FRAG_POLICY_BSD         3
#define FRAG_POLICY_BSD_RIGHT   4
#define FRAG_POLICY_LAST        5
/* Combo of FIRST & LAST, depending on overlap situation. */
#define FRAG_POLICY_WINDOWS     6
/* Combo of FIRST & LAST, depending on overlap situation. */
#define FRAG_POLICY_SOLARIS     7
#define FRAG_POLICY_DEFAULT     FRAG_POLICY_BSD

/* max packet size */
#define DATASIZE (ETHERNET_HEADER_LEN+IP_MAXPACKET)

/* max frags in a single frag tracker */
#define DEFAULT_MAX_FRAGS   8192

/*max preallocated frags */
#define MAX_PREALLOC_FRAGS 50000
#define MIN_FRAG_MEMCAP 16384

/* return values for CheckTimeout() */
#define FRAG_TIME_OK            0
#define FRAG_TIMEOUT            1

/* return values for Frag3Insert() */
#define FRAG_INSERT_OK          0
#define FRAG_INSERT_FAILED      1
#define FRAG_INSERT_REJECTED    2
#define FRAG_INSERT_TIMEOUT     3
#define FRAG_INSERT_ATTACK      4
#define FRAG_INSERT_ANOMALY     5
#define FRAG_INSERT_TTL         6
#define FRAG_INSERT_OVERLAP_LIMIT  7

/* return values for Frag3CheckFirstLast() */
#define FRAG_FIRSTLAST_OK       0
#define FRAG_LAST_DUPLICATE     1

/* return values for Frag3Expire() */
#define FRAG_OK                 0
#define FRAG_TRACKER_TIMEOUT    1
#define FRAG_LAST_OFFSET_ADJUST 2

/* flag for detecting attacks/alerting */
#define FRAG3_DETECT_ANOMALIES  0x01

/*  D A T A   S T R U C T U R E S  **********************************/

/* runtime context for a specific instance of an engine */
typedef struct _Frag3Context
{
    uint16_t   frag_policy;  /* policy to use for target-based reassembly */
    uint32_t   frag_timeout; /* timeout for frags in this policy */

    uint8_t    min_ttl;       /* Minimum TTL to accept */

    char        frag3_alerts;      /* Whether or not frag3 alerts are enabled */

    IpAddrSet  *bound_addrs; /* addresses bound to this context */

    /**limit on number of fragments before excessive fragmentation event is generated.
     */
    uint32_t   overlap_limit;

    /**Fragment that is too small to be legal
     */
    uint32_t   min_fragment_length;

} Frag3Context;

/* struct to manage an individual fragment */
typedef struct _Frag3Frag
{
    uint8_t   *data;     /* ptr to adjusted start position */
    uint16_t   size;     /* adjusted frag size */
    uint16_t   offset;   /* adjusted offset position */

    uint8_t   *fptr;     /* free pointer */
    uint16_t   flen;     /* free len, unneeded? */

    struct _Frag3Frag *prev;
    struct _Frag3Frag *next;

    int         ord;
    char        last;
} Frag3Frag;

typedef struct _fragkey
{
    uint32_t   sip[4];
    uint32_t   dip[4];
    uint32_t   id;
    uint16_t   vlan_tag;
    uint8_t    proto;         /* IP protocol, unused for IPv6 */
    uint8_t    ipver;         /* Version */
#ifdef MPLS
    uint32_t   mlabel;
    /* For 64 bit alignment since this is allocated in front of a FragTracker
     * and the structures are laid on top of that allocated memory */
#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
    uint16_t   address_space_id_src;
    uint16_t   address_space_id_dst;
#else
    uint16_t   addressSpaceId;
    uint16_t   addressSpaceIdPad1;
#endif
#else
    uint32_t   mpad;
#endif
#else
#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
    uint16_t   address_space_id_src;
    uint16_t   address_space_id_dst;
#else
    uint16_t   addressSpaceId;
    uint16_t   addressSpaceIdPad1;
#endif
    uint32_t   addressSpaceIdPad2;
#endif
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
    uint32_t   carrierId;
#endif
} FRAGKEY;

/* Only track a certain number of alerts per session */
#define MAX_FRAG_ALERTS  8

/* global configuration data struct for this preprocessor */
typedef struct _Frag3Config
{
    int disabled;
    uint32_t   max_frags;            /* max frags to track */
    unsigned long memcap;            /* memcap for frag3 */
    int ten_percent;                 /* holder for self preservation data */
    uint32_t   static_frags;         /* static frag nodes to keep around */
    uint8_t    use_prealloc;         /* flag to indicate prealloc nodes in use */
    uint8_t    use_prealloc_frags;   /* flag to indicate prealloc nodes in use */
    Frag3Context *default_context;
    Frag3Context **frag3ContextList;  /* List of Frag3 Contexts configured */
    uint8_t numFrag3Contexts;
    uint32_t ref_count;

} Frag3Config;

/* tracker for a fragmented packet set */
typedef struct _FragTracker
{
    uint32_t sip[4];
    uint32_t dip[4];
    uint32_t id;           /* IP ID */
    uint8_t protocol;      /* IP protocol */
    uint8_t ipver;         /* Version */

    uint8_t ttl;           /* ttl used to detect evasions */
    uint8_t alerted;
    uint32_t frag_flags;   /* bit field */

    uint32_t frag_bytes;   /* number of fragment bytes stored, based
                             * on aligned fragment offsets/sizes
                             */

    uint32_t calculated_size; /* calculated size of reassembled pkt, based on
                                * last frag offset
                                */

    uint32_t frag_pkts;   /* nummber of frag pkts stored under this tracker */

    struct timeval frag_time; /* time we started tracking this frag */

    Frag3Frag *fraglist;      /* list of fragments */
    Frag3Frag *fraglist_tail; /* tail ptr for easy appending */
    int fraglist_count;       /* handy dandy counter */

    uint32_t alert_gid[MAX_FRAG_ALERTS]; /* flag alerts seen in a frag list  */
    uint32_t alert_sid[MAX_FRAG_ALERTS]; /* flag alerts seen in a frag list  */
    uint8_t  alert_count;                /* count alerts seen in a frag list */

    uint32_t ip_options_len;  /* length of ip options for this set of frags */
    uint32_t ip_option_count; /* number of ip options for this set of frags */
    uint8_t *ip_options_data; /* ip options from offset 0 packet */

    uint32_t copied_ip_options_len;  /* length of 'copied' ip options */
    uint32_t copied_ip_option_count; /* number of 'copied' ip options */

    Frag3Context *context;

    int ordinal;
#ifdef TARGET_BASED
    int ipprotocol;
    int application_protocol;
#endif
    uint32_t frag_policy;
    /**Count of IP fragment overlap for each packet id.
     */
    uint32_t overlap_count;

    /* Configuration in use when this tracker was created */
    tSfPolicyId policy_id;
    tSfPolicyUserContextId config;

} FragTracker;

/* statistics tracking struct */
typedef struct _Frag3Stats
{
    uint32_t  total;
    uint32_t  overlaps;
    uint32_t  reassembles;
    uint32_t  prunes;
    uint32_t  timeouts;
    uint32_t  fragtrackers_created;
    uint32_t  fragtrackers_released;
    uint32_t  fragtrackers_autoreleased;
    uint32_t  fragnodes_created;
    uint32_t  fragnodes_released;
    uint32_t  discards;
    uint32_t  anomalies;
    uint32_t  alerts;
    uint32_t  drops;

} Frag3Stats;


/*  G L O B A L S  **************************************************/
static tSfPolicyUserContextId frag3_config = NULL;  /* current configuration */

/* Config to use to evaluate
 * If a frag tracker is found in the hash table, the configuration under
 * which it was created will be used */
static Frag3Config *frag3_eval_config = NULL;

static SFXHASH *f_cache = NULL;                 /* fragment hash table */
static Frag3Frag *prealloc_frag_list = NULL;    /* head for prealloc queue */

static unsigned long frag3_mem_in_use = 0;            /* memory in use, used for self pres */

static uint32_t prealloc_nodes_in_use;  /* counter for debug */

static Frag3Stats f3stats;               /* stats struct */

static Packet* defrag_pkt = NULL;
#ifdef GRE
static Packet* encap_defrag_pkt = NULL;
#endif

static uint32_t pkt_snaplen = 0;

static uint32_t old_static_frags;
static unsigned long hashTableSize;
static unsigned long fcache_new_memcap;

/* enum for policy names */
static char *frag_policy_names[] = { "no policy!",
    "FIRST",
    "LINUX",
    "BSD",
    "BSD_RIGHT",
    "LAST",
    "WINDOWS",
    "SOLARIS"};

#ifdef PERF_PROFILING
PreprocStats frag3PerfStats;
PreprocStats frag3InsertPerfStats;
PreprocStats frag3RebuildPerfStats;
#endif

/*
 * external globals for startup
 */
extern char *file_name;
extern int file_line;


/*  P R O T O T Y P E S  ********************************************/
static void Frag3ParseGlobalArgs(Frag3Config *, char *);
static void Frag3ParseArgs(struct _SnortConfig *, char *, Frag3Context *);
static inline int Frag3Expire(Packet *, FragTracker *, Frag3Context *);
static FragTracker *Frag3GetTracker(Packet *, FRAGKEY *);
static int Frag3NewTracker(Packet *p, FRAGKEY *fkey, Frag3Context *);
static int Frag3Insert(Packet *, FragTracker *, FRAGKEY *, Frag3Context *);
static void Frag3Rebuild(FragTracker *, Packet *);
static inline int Frag3IsComplete(FragTracker *);
static int Frag3HandleIPOptions(FragTracker *, Packet *);
static void Frag3PrintStats(int);
static void Frag3FreeConfig(Frag3Config *);
static void Frag3FreeConfigs(tSfPolicyUserContextId);

#ifdef SNORT_RELOAD
static void Frag3ReloadGlobal(struct _SnortConfig *, char *, void **);
static void Frag3ReloadEngine(struct _SnortConfig *, char *, void **);
static int Frag3ReloadVerify(struct _SnortConfig *, void *);
static void * Frag3ReloadSwap(struct _SnortConfig *, void *);
static void Frag3ReloadSwapFree(void *);
static uint32_t Frag3MemReloadAdjust(unsigned);
static bool Frag3ReloadAdjust(bool, tSfPolicyId, void *);
#endif

/* deletion funcs */
static int Frag3Prune(FragTracker *);
static struct timeval *pkttime;    /* packet timestamp */
static void Frag3DeleteFrag(Frag3Frag *);
static void Frag3RemoveTracker(void *, void *);
static void Frag3DeleteTracker(FragTracker *);
static int Frag3AutoFree(void *, void *);
static int Frag3UserFree(void *, void *);

/* fraglist handler funcs */
static inline void Frag3FraglistAddNode(FragTracker *, Frag3Frag *, Frag3Frag *);
static inline void Frag3FraglistDeleteNode(FragTracker *, Frag3Frag *);

/* prealloc queue handler funcs */
static inline Frag3Frag *Frag3PreallocPop();
static inline void Frag3PreallocPush(Frag3Frag *);

/* main preprocessor functions */
static void Frag3Defrag(Packet *, void *);
static void Frag3CleanExit(int, void *);
static void Frag3Reset(int, void *);
static void Frag3ResetStats(int, void *);
static void Frag3Init(struct _SnortConfig *, char *);
static void Frag3GlobalInit(struct _SnortConfig *, char *);
static int Frag3VerifyConfig(struct _SnortConfig *);
static void Frag3PostConfigInit(struct _SnortConfig *, void *);

char *FragIPToStr(uint32_t ip[4], uint8_t proto)
{
    char *ret_str;
    sfaddr_t srcip;
    sfip_set_raw(&srcip, ip, proto == 4 ? AF_INET : AF_INET6);

    ret_str = sfip_to_str(&srcip);
    return ret_str;
}

#ifdef DEBUG_FRAG3
/**
 * Print out a FragTracker structure
 *
 * @param ft Pointer to the FragTracker to print
 *
 * @return none
 */
static void PrintFragTracker(FragTracker *ft)
{
    LogMessage("FragTracker %p\n", ft);
    if(ft)
    {
        LogMessage("        sip: %s\n", FragIPToStr(ft->sip, ft->ipver));
        LogMessage("        dip: %s\n", FragIPToStr(ft->dip, ft->ipver));
        LogMessage("         id: %d\n", ft->id);
        LogMessage("      proto: 0x%X\n", ft->protocol);
        LogMessage("      ipver: 0x%X\n", ft->ipver);
        LogMessage("        ttl: %d\n", ft->ttl);
        LogMessage("    alerted: %d\n", ft->alerted);
        LogMessage(" frag_flags: 0x%X\n", ft->frag_flags);
        LogMessage(" frag_bytes: %d\n", ft->frag_bytes);
        LogMessage("  calc_size: %d\n", ft->calculated_size);
        LogMessage("  frag_pkts: %d\n", ft->frag_pkts);
        LogMessage("  frag_time: %lu %lu\n", ft->frag_time.tv_sec,
                ft->frag_time.tv_usec);
        LogMessage("   fraglist: %p\n", ft->fraglist);
        LogMessage("    fl_tail: %p\n", ft->fraglist_tail);
        LogMessage("fraglst cnt: %d\n", ft->fraglist_count);
    }
}

/**
 * Print out a FragKey structure
 *
 * @param fkey Pointer to the FragKey to print
 *
 * @return none
 */
static void PrintFragKey(FRAGKEY *fkey)
{
    LogMessage("FragKey %p\n", fkey);

    if(fkey)
    {
        LogMessage("   sip: %s\n", FragIPToStr(fkey->sip, fkey->ipver));
        LogMessage("   dip: %s\n", FragIPToStr(fkey->dip, fkey->ipver));
        LogMessage("     id: %d\n", fkey->id);
        LogMessage("  proto: 0x%X\n", fkey->proto);
#ifdef MPLS
        LogMessage(" mlabel: 0x%08X\n", fkey->mlabel);
#endif
#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
        LogMessage(" addr id: %d\n", fkey->addressSpaceId);
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
        LogMessage(" carrier id: %u\n", fkey->carrierId);
#endif
    }
}

/**
 * Print out a Frag3Frag structure
 *
 * @param f Pointer to the Frag3Frag to print
 *
 * @return none
 */
static void PrintFrag3Frag(Frag3Frag *f)
{
    LogMessage("Frag3Frag: %p\n", f);

    if(f)
    {
        LogMessage("    data: %p\n", f->data);
        LogMessage("    size: %d\n", f->size);
        LogMessage("  offset: %d\n", f->offset);
        LogMessage("    fptr: %p\n", f->fptr);
        LogMessage("    flen: %d\n", f->flen);
        LogMessage("    prev: %p\n", f->prev);
        LogMessage("    next: %p\n", f->next);
    }
}

#endif  /* DEBUG_FRAG3 */

/**
 * Print out the global runtime configuration
 *
 * @param None
 *
 * @return none
 */
static void Frag3PrintGlobalConfig(Frag3Config *gconfig)
{
    if (gconfig == NULL)
        return;

    LogMessage("Frag3 global config:\n");
    if(gconfig->disabled)
    {
        LogMessage("      Frag3: INACTIVE\n");
    }
    LogMessage("    Max frags: %d\n", gconfig->max_frags);
    if(!gconfig->use_prealloc)
        LogMessage("    Fragment memory cap: %lu bytes\n",
                gconfig->memcap);
    else
    {
        if (gconfig->static_frags)
            LogMessage("    Preallocated frag nodes: %u\n",
                gconfig->static_frags);
        if (!gconfig->use_prealloc_frags)
            LogMessage("    Memory cap used to determine preallocated frag nodes: %lu\n",
                    gconfig->memcap);
    }
}


/**
 * Print out a defrag engine runtime context
 *
 * @param context Pointer to the context structure to print
 *
 * @return none
 */
static void Frag3PrintEngineConfig(Frag3Context *context)
{

    LogMessage("Frag3 engine config:\n");
    if (context->bound_addrs != NULL)
    {
        IpAddrSetPrint("    Bound Addresses: ", context->bound_addrs);
    }
    else
    {
        LogMessage("    Bound Address: default\n");
    }
    LogMessage("    Target-based policy: %s\n",
            frag_policy_names[context->frag_policy]);
    LogMessage("    Fragment timeout: %d seconds\n",
            context->frag_timeout);
    LogMessage("    Fragment min_ttl:   %d\n", context->min_ttl);
    LogMessage("    Fragment Anomalies: %s\n",
            context->frag3_alerts ? "Alert" : "No Alert");

    LogMessage("    Overlap Limit:     %d\n",
            context->overlap_limit);
    LogMessage("    Min fragment Length:     %d\n",
            context->min_fragment_length);
}

/**
 * Generate an event due to IP options being detected in a frag packet
 *
 * @param context Current run context
 *
 * @return none
 */
static inline void EventAnomIpOpts(Frag3Context *context)
{
    if(!(context->frag3_alerts & FRAG3_DETECT_ANOMALIES))
        return;

    SnortEventqAdd(GENERATOR_SPP_FRAG3,     /* GID */
            FRAG3_IPOPTIONS,         /* SID */
            1,                       /* rev */
            0,                       /* classification enum */
            3,                       /* priority (low) */
            FRAG3_IPOPTIONS_STR,     /* event message */
            NULL);                   /* rule info ptr */

   f3stats.alerts++;
}

/**
 * Generate an event due to a Teardrop-style attack detected in a frag packet
 *
 * @param context Current run context
 *
 * @return none
 */
static inline void EventAttackTeardrop(Frag3Context *context)
{
    if(!(context->frag3_alerts & FRAG3_DETECT_ANOMALIES))
        return;

    SnortEventqAdd(GENERATOR_SPP_FRAG3,     /* GID */
            FRAG3_TEARDROP,          /* SID */
            1,                       /* rev */
            0,                       /* classification enum */
            3,                       /* priority (low) */
            FRAG3_TEARDROP_STR,      /* event message */
            NULL);                   /* rule info ptr */

   f3stats.alerts++;
}

/**
 * Generate an event for very small fragment
 *
 * @param context Current run context
 *
 * @return none
 */
static inline void EventTinyFragments(Frag3Context *context)
{
    if(!(context->frag3_alerts & FRAG3_DETECT_ANOMALIES))
        return;

    SnortEventqAdd(GENERATOR_SPP_FRAG3,     /* GID */
            FRAG3_TINY_FRAGMENT,          /* SID */
            1,                       /* rev */
            0,                       /* classification enum */
            3,                       /* priority (low) */
            FRAG3_TINY_FRAGMENT_STR, /* event message */
            NULL);                   /* rule info ptr */

   f3stats.alerts++;
}

/**
 * Generate an event due to excessive fragment overlap detected in a frag packet
 *
 * @param context Current run context
 *
 * @return none
 */
static inline void EventExcessiveOverlap(Frag3Context *context)
{
    //@TBD dschahal do I need this
    if(!(context->frag3_alerts & FRAG3_DETECT_ANOMALIES))
        return;

    SnortEventqAdd(GENERATOR_SPP_FRAG3,     /* GID */
            FRAG3_EXCESSIVE_OVERLAP,          /* SID */
            1,                       /* rev */
            0,                       /* classification enum */
            3,                       /* priority (low) */
            FRAG3_EXCESSIVE_OVERLAP_STR, /* event message */
            NULL);                   /* rule info ptr */

   f3stats.alerts++;
}

/**
 * Generate an event due to a fragment being too short, typcially based
 * on a non-last fragment that doesn't properly end on an 8-byte boundary
 *
 * @param context Current run context
 *
 * @return none
 */
static inline void EventAnomShortFrag(Frag3Context *context)
{
    if(!(context->frag3_alerts & FRAG3_DETECT_ANOMALIES))
        return;

    SnortEventqAdd(GENERATOR_SPP_FRAG3,   /* GID */
            FRAG3_SHORT_FRAG,             /* SID */
            1,                            /* rev */
            0,                            /* classification enum */
            3,                            /* priority (low) */
            FRAG3_SHORT_FRAG_STR,         /* event message */
            NULL);                        /* rule info ptr */

   f3stats.alerts++;
   f3stats.anomalies++;
}

/**
 * This fragment's size will end after the already calculated reassembled
 * fragment end, as in a Bonk/Boink/etc attack.
 *
 * @param context Current run context
 *
 * @return none
 */
static inline void EventAnomOversize(Frag3Context *context)
{
    if(!(context->frag3_alerts & FRAG3_DETECT_ANOMALIES))
        return;

    SnortEventqAdd(GENERATOR_SPP_FRAG3,/* GID */
            FRAG3_ANOMALY_OVERSIZE,  /* SID */
            1,                       /* rev */
            0,                       /* classification enum */
            3,                       /* priority (low) */
            FRAG3_ANOM_OVERSIZE_STR, /* event message */
            NULL);                   /* rule info ptr */

   f3stats.alerts++;
   f3stats.anomalies++;
}

/**
 * The current fragment will be inserted with a size of 0 bytes, that's
 * an anomaly if I've ever seen one.
 *
 * @param context Current run context
 *
 * @return none
 */
static inline void EventAnomZeroFrag(Frag3Context *context)
{
    if(!(context->frag3_alerts & FRAG3_DETECT_ANOMALIES))
        return;

    SnortEventqAdd(GENERATOR_SPP_FRAG3,/* GID */
            FRAG3_ANOMALY_ZERO,      /* SID */
            1,                       /* rev */
            0,                       /* classification enum */
            3,                       /* priority (low) */
            FRAG3_ANOM_ZERO_STR,     /* event message */
            NULL);                   /* rule info ptr */

   f3stats.alerts++;
   f3stats.anomalies++;
}

/**
 * The reassembled packet will be bigger than 64k, generate an event.
 *
 * @param context Current run context
 *
 * @return none
 */
static inline void EventAnomBadsizeLg(Frag3Context *context)
{
    if(!(context->frag3_alerts & FRAG3_DETECT_ANOMALIES))
        return;

    SnortEventqAdd(GENERATOR_SPP_FRAG3,/* GID */
            FRAG3_ANOMALY_BADSIZE_LG,   /* SID */
            1,                       /* rev */
            0,                       /* classification enum */
            3,                       /* priority (low) */
            FRAG3_ANOM_BADSIZE_LG_STR,  /* event message */
            NULL);                   /* rule info ptr */

   f3stats.alerts++;
   f3stats.anomalies++;
}

/**
 * Fragment size is negative after insertion (end < offset).
 *
 * @param context Current run context
 *
 * @return none
 */
static inline void EventAnomBadsizeSm(Frag3Context *context)
{
    if(!(context->frag3_alerts & FRAG3_DETECT_ANOMALIES))
        return;

    SnortEventqAdd(GENERATOR_SPP_FRAG3,/* GID */
            FRAG3_ANOMALY_BADSIZE_SM,  /* SID */
            1,                         /* rev */
            0,                         /* classification enum */
            3,                         /* priority (low) */
            FRAG3_ANOM_BADSIZE_SM_STR, /* event message */
            NULL);                     /* rule info ptr */

   f3stats.alerts++;
   f3stats.anomalies++;
}

/**
 * There is an overlap with this fragment, someone is probably being naughty.
 *
 * @param context Current run context
 *
 * @return none
 */
static inline void EventAnomOverlap(Frag3Context *context)
{
    if(!(context->frag3_alerts & FRAG3_DETECT_ANOMALIES))
        return;

    SnortEventqAdd(GENERATOR_SPP_FRAG3,/* GID */
            FRAG3_ANOMALY_OVLP,   /* SID */
            1,                    /* rev */
            0,                    /* classification enum */
            3,                    /* priority (low) */
            FRAG3_ANOM_OVLP_STR,  /* event message */
            NULL);                /* rule info ptr */

   f3stats.alerts++;
   f3stats.anomalies++;
}

/**
 * Generate an event due to TTL below the configured minimum
 *
 * @param context Current run context
 *
 * @return none
 */
static inline void EventAnomScMinTTL(Frag3Context *context)
{
    if(!(context->frag3_alerts & FRAG3_DETECT_ANOMALIES))
        return;

    SnortEventqAdd(GENERATOR_SPP_FRAG3, /* GID */
            FRAG3_MIN_TTL_EVASION,   /* SID */
            1,                       /* rev */
            0,                       /* classification enum */
            3,                       /* priority (low) */
            FRAG3_MIN_TTL_EVASION_STR,  /* event message */
            NULL);                   /* rule info ptr */

   f3stats.alerts++;
}

int frag3_print_mem_stats(FILE *fd, char* buffer, PreprocMemInfo *meminfo)
{
    int len = 0;
    time_t curr_time;

    if (fd)
    {
        len = fprintf(fd, ",%lu,%u"
                 ",%lu,%u,%u"
                 ",%lu,%u,%u,%lu"
                 , frag3_mem_in_use
                 , prealloc_nodes_in_use
                 , meminfo[PP_MEM_CATEGORY_SESSION].used_memory
                 , meminfo[PP_MEM_CATEGORY_SESSION].num_of_alloc
                 , meminfo[PP_MEM_CATEGORY_SESSION].num_of_free
                 , meminfo[PP_MEM_CATEGORY_CONFIG].used_memory
                 , meminfo[PP_MEM_CATEGORY_CONFIG].num_of_alloc
                 , meminfo[PP_MEM_CATEGORY_CONFIG].num_of_free
                 , meminfo[PP_MEM_CATEGORY_SESSION].used_memory +
                   meminfo[PP_MEM_CATEGORY_CONFIG].used_memory);

        return len;
    }

    curr_time = time(NULL); 

    if (buffer)
    {
        len = snprintf(buffer, CS_STATS_BUF_SIZE, "\n\nMemory Statistics of Frag3 on: %s\n"
            "    Memory in use         : %lu\n"
            "    prealloc nodes in use : %u\n\n"
            , ctime(&curr_time)
            , frag3_mem_in_use
            , prealloc_nodes_in_use);
    } else {
        LogMessage("\n");
        LogMessage("Memory Statistics of Frag3 on: %s\n", ctime(&curr_time));
        LogMessage("    Memory in use         : %lu\n", frag3_mem_in_use); 
        LogMessage("    prealloc nodes in use : %u\n\n", prealloc_nodes_in_use);
    }

    return len;
}

/**
 * Main setup function to register frag3 with the rest of Snort.
 *
 * @param none
 *
 * @return none
 */
void SetupFrag3(void)
{
    RegisterMemoryStatsFunction(PP_FRAG3, frag3_print_mem_stats);
#ifndef SNORT_RELOAD
    RegisterPreprocessor("frag3_global", Frag3GlobalInit);
    RegisterPreprocessor("frag3_engine", Frag3Init);
#else
    RegisterPreprocessor("frag3_global", Frag3GlobalInit,
                         Frag3ReloadGlobal, Frag3ReloadVerify, Frag3ReloadSwap,
                         Frag3ReloadSwapFree);
    RegisterPreprocessor("frag3_engine", Frag3Init, Frag3ReloadEngine,
                         NULL, NULL, NULL);
#endif

    DEBUG_WRAP(DebugMessage(DEBUG_FRAG, "Preprocessor: frag3 is setup...\n"););
}

uint32_t Frag3KeyHashFunc(SFHASHFCN *p, unsigned char *d, int n)
{
    uint32_t a,b,c;
    uint32_t offset = 0;
#ifdef MPLS
    uint32_t tmp = 0;
#endif
#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
    uint32_t tmp2 = 0;
#endif

    a = *(uint32_t *)d;        /* IPv6 sip[0] */
    b = *(uint32_t *)(d+4);    /* IPv6 sip[1] */
    c = *(uint32_t *)(d+8);    /* IPv6 sip[2] */
    mix(a,b,c);

    a += *(uint32_t *)(d+12);  /* IPv6 sip[3] */
    b += *(uint32_t *)(d+16);  /* IPv6 dip[0] */
    c += *(uint32_t *)(d+20);  /* IPv6 dip[1] */
    mix(a,b,c);

    a += *(uint32_t *)(d+24);  /* IPv6 dip[2] */
    b += *(uint32_t *)(d+28);  /* IPv6 dip[3] */
    c += *(uint32_t *)(d+32);  /* IPv6 id */
    mix(a,b,c);

    offset = 36;

    a += *(uint32_t *)(d+offset);  /* vlan, proto, ipver */
#ifdef MPLS
    tmp = *(uint32_t*)(d+offset+4);
    if( tmp )
    {
        b += tmp;   /* mpls label */
    }
    offset += 8;    /* skip past vlan/proto/ipver & mpls label */
#else
    offset += 4;    /* skip past vlan/proto/ipver */
#endif

#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
    tmp2 = *(uint32_t*)(d+offset); /* after offset that has been moved */
    c += tmp2; /* address space id and 16bits of zero'd pad */
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
    mix(a,b,c);
    a += *(uint32_t*)(d+offset+4);
#endif

    final(a,b,c);

    return c;
}

int Frag3KeyCmpFunc(const void *s1, const void *s2, size_t n)
{
#ifndef SPARCV9 /* ie, everything else, use 64bit comparisons */
    uint64_t *a, *b;

    a = (uint64_t*)s1;
    b = (uint64_t*)s2;
    if (*a - *b) return 1;      /* Compares IPv4 sip/dip */
                                /* Compares IPv6 sip[0,1] */
    a++;
    b++;
    if (*a - *b) return 1;      /* Compares IPv6 sip[2,3] */

    a++;
    b++;
    if (*a - *b) return 1;      /* Compares IPv6 dip[0,1] */

    a++;
    b++;
    if (*a - *b) return 1;      /* Compares IPv6 dip[2,3] */

    a++;
    b++;
    if (*a - *b) return 1;      /* Compares IPv4 id/pad, vlan/proto/ipver */
                                /* Compares IPv6 id, vlan/proto/ipver */

#ifdef MPLS
    a++;
    b++;
#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
    if (*a - *b) return 1;      /* Compares MPLS label, AddressSpace ID and 16bit pad */
#else
    {
        uint32_t *x, *y;
        x = (uint32_t *)a;
        y = (uint32_t *)b;
        //x++;
        //y++;
        if (*x - *y) return 1;  /* Compares mpls label, no pad */
    }
#endif
#else
#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
    a++;
    b++;
    {
        uint16_t *x, *y;
        x = (uint16_t *)a;
        y = (uint16_t *)b;
        //x++;
        //y++;
        if (*x - *y) return 1;  /* Compares addressSpaceID, no pad */
    }
#endif
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
    a++;
    b++;
    {
        uint32_t *x, *y;
        x = (uint32_t *)a;
        y = (uint32_t *)b;
        if (*x - *y) return 1; /* Compares carrierID */
    }
#endif

#else /* SPARCV9 */
    uint32_t *a,*b;

    a = (uint32_t*)s1;
    b = (uint32_t*)s2;
    if ((*a - *b) || (*(a+1) - *(b+1))) return 1;       /* Compares IPv4 sip/dip */
                                /* Compares IPv6 sip[0,1] */
    a+=2;
    b+=2;
    if ((*a - *b) || (*(a+1) - *(b+1))) return 1;       /* Compares IPv6 sip[2,3] */

    a+=2;
    b+=2;
    if ((*a - *b) || (*(a+1) - *(b+1))) return 1;       /* Compares IPv6 dip[0,1] */

    a+=2;
    b+=2;
    if ((*a - *b) || (*(a+1) - *(b+1))) return 1;       /* Compares IPv6 dip[2,3] */

    a+=2;
    b+=2;
    if ((*a - *b) || (*(a+1) - *(b+1))) return 1;       /* Compares IPv4 id/pad, vlan/proto/ipver */
                                /* Compares IPv6 id, vlan/proto/ipver */

#ifdef MPLS
    a+=2;
    b+=2;
    {
        uint32_t *x, *y;
        x = (uint32_t *)a;
        y = (uint32_t *)b;
        //x++;
        //y++;
        if (*x - *y) return 1;  /* Compares mpls label */
    }
#endif
#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
#ifdef MPLS
    a++;
    b++;
#else
    a+=2;
    b+=2;
#endif
    {
        uint16_t *x, *y;
        x = (uint16_t *)a;
        y = (uint16_t *)b;
        //x++;
        //y++;
        if (*x - *y) return 1;  /* Compares addressSpaceID, no pad */
    }
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
    a++;
    b++;
    {
        uint32_t *x, *y;
        x = (uint32_t *)a;
        y = (uint32_t *)b;
        if (*x - *y) return 1; /* Compares carrierID */
    }
#endif
#endif /* SPARCV9 */

    return 0;
}

/**
 * Global init function, handles setting up the runtime hash table and
 * memory management mode. Global configuration applies only to default configuration,
 * which is in vlanGroup 0
 *
 * @param args argument string to process for config data
 *
 * @return none
 */
static void Frag3GlobalInit(struct _SnortConfig *sc, char *args)
{
    Frag3Config *pCurrentPolicyConfig = NULL;
    Frag3Config *pDefaultPolicyConfig = NULL;
    tSfPolicyId policy_id = getParserPolicy(sc);

    if (frag3_config == NULL)
    {
        //create a context
        frag3_config = sfPolicyConfigCreate();

        defrag_pkt = Encode_New();
#ifdef GRE
        encap_defrag_pkt = Encode_New();
#endif

#ifdef PERF_PROFILING
        RegisterPreprocessorProfile("frag3", &frag3PerfStats, 0, &totalPerfStats, NULL);
        RegisterPreprocessorProfile("frag3insert", &frag3InsertPerfStats, 1, &frag3PerfStats, NULL);
        RegisterPreprocessorProfile("frag3rebuild", &frag3RebuildPerfStats, 1, &frag3PerfStats, NULL);
#endif

        AddFuncToPreprocCleanExitList(Frag3CleanExit, NULL, PP_FRAG3_PRIORITY, PP_FRAG3);
        AddFuncToPreprocResetList(Frag3Reset, NULL, PP_FRAG3_PRIORITY, PP_FRAG3);
        AddFuncToPreprocResetStatsList(Frag3ResetStats, NULL, PP_FRAG3_PRIORITY, PP_FRAG3);
        AddFuncToConfigCheckList(sc, Frag3VerifyConfig);
        AddFuncToPreprocPostConfigList(sc, Frag3PostConfigInit, NULL);
        RegisterPreprocStats("frag3", Frag3PrintStats);
    }

    sfPolicyUserPolicySet (frag3_config, policy_id);
    pCurrentPolicyConfig = (Frag3Config *)sfPolicyUserDataGetCurrent(frag3_config);
    pDefaultPolicyConfig = (Frag3Config *)sfPolicyUserDataGetDefault(frag3_config);

    if ((policy_id != getDefaultPolicy()) && (pDefaultPolicyConfig == NULL))
    {
        ParseError("Frag3: Must configure default policy if other policies "
                   "are going to be used.\n");
    }

    if (pCurrentPolicyConfig != NULL)
    {
        FatalError("%s(%d) The frag3 global configuration can only be "
                   "configured once.\n", file_name, file_line);
    }

    pCurrentPolicyConfig = (Frag3Config *)SnortPreprocAlloc(1, sizeof(Frag3Config),
                              PP_FRAG3, PP_MEM_CATEGORY_CONFIG);
    sfPolicyUserDataSetCurrent(frag3_config, pCurrentPolicyConfig);

    /* setup default values */
    pCurrentPolicyConfig->max_frags = DEFAULT_MAX_FRAGS;
    pCurrentPolicyConfig->memcap = FRAG_MEMCAP;
    pCurrentPolicyConfig->static_frags = 0;
    pCurrentPolicyConfig->use_prealloc = 0;
    pCurrentPolicyConfig->use_prealloc_frags = 0;

    Frag3ParseGlobalArgs(pCurrentPolicyConfig, args);

    if (policy_id != getDefaultPolicy())
    {
        /* Can't set these in alternate policies */
        pCurrentPolicyConfig->memcap = pDefaultPolicyConfig->memcap;
        pCurrentPolicyConfig->max_frags = pDefaultPolicyConfig->max_frags;
        pCurrentPolicyConfig->use_prealloc = pDefaultPolicyConfig->use_prealloc;
        pCurrentPolicyConfig->use_prealloc_frags = pDefaultPolicyConfig->use_prealloc_frags;
        pCurrentPolicyConfig->static_frags = pDefaultPolicyConfig->static_frags;
    }

    /*
     * we really only need one frag cache no matter how many different
     * contexts we have loaded
     */
    if(f_cache == NULL)
    {
        /* we keep FragTrackers in the hash table.. */
        hashTableSize = (unsigned long) (pCurrentPolicyConfig->max_frags * 1.4);
        unsigned long maxFragMem = pCurrentPolicyConfig->max_frags * (
                            sizeof(FragTracker) +
                            sizeof(SFXHASH_NODE) +
                            sizeof (FRAGKEY) +
                            sizeof(SFXHASH_NODE *));
        unsigned long tableMem = (hashTableSize + 1) * sizeof(SFXHASH_NODE *);
        unsigned long maxMem = maxFragMem + tableMem;
        f_cache = sfxhash_new(
                hashTableSize,       /* number of hash buckets */
                sizeof(FRAGKEY),     /* size of the key we're going to use */
                sizeof(FragTracker), /* size of the storage node */
                maxMem,              /* memcap for frag trackers */
                1,                   /* use auto node recovery */
                Frag3AutoFree,       /* anr free function */
                Frag3UserFree,       /* user free function */
                1);                  /* recycle node flag */

        /* can't proceed if we can't get a fragment cache */
        if(!f_cache)
        {
            LogMessage("WARNING: Unable to generate new sfxhash for frag3, "
                       "defragmentation disabled.\n");
            return;
        }

        sfxhash_set_keyops(f_cache, Frag3KeyHashFunc, Frag3KeyCmpFunc);
    }

    /* display the global config for the user */
    Frag3PrintGlobalConfig(pCurrentPolicyConfig);

#ifdef REG_TEST
    LogMessage("\n");
    LogMessage("    FragTracker Size: %lu\n",(unsigned long)sizeof(FragTracker));
    LogMessage("\n");
#endif

    /* register the preprocessor func node */
    if ( !pCurrentPolicyConfig->disabled )
    {
        AddFuncToPreprocList(sc, Frag3Defrag, PP_FRAG3_PRIORITY, PP_FRAG3, PROTO_BIT__IP);
        session_api->enable_preproc_all_ports( sc, PP_FRAG3, PROTO_BIT__IP );
    }
}

/**
 * Setup a frag3 engine context
 *
 * @param args list of configuration arguments
 *
 * @return none
 */
static void Frag3Init(struct _SnortConfig *sc, char *args)
{
    Frag3Context *context;      /* context pointer */
    tSfPolicyId policy_id = getParserPolicy(sc);
    Frag3Config *config = NULL;

    DEBUG_WRAP(DebugMessage(DEBUG_FRAG, "Initializing frag3\n"););

    config = (Frag3Config *)sfPolicyUserDataGet(frag3_config, policy_id);

    if (config == NULL)
    {
        FatalError("[!] Unable to configure frag3 engine!\n"
                "Frag3 global config has not been established, "
                "please issue a \"preprocessor frag3_global\" directive\n");
        return;
    }


    /*
     * setup default context config.  Thinking maybe we should go with
     * FRAG_POLICY_FIRST or FRAG_POLICY_LINUX as the default instead of
     * BSD since Win32/Linux have a higher incidence of occurrence.  Anyone
     * with an opinion on the matter feel free to email me...
     */
    context = (Frag3Context *) SnortPreprocAlloc(1, sizeof(Frag3Context),
                          PP_FRAG3, PP_MEM_CATEGORY_CONFIG);
    context->frag_policy = FRAG_POLICY_DEFAULT;
    context->frag_timeout = FRAG_PRUNE_QUANTA; /* 60 seconds */
    context->min_ttl = FRAG3_MIN_TTL;
    context->frag3_alerts = 0;

    /* parse the configuration for this engine */
    Frag3ParseArgs(sc, args, context);

    if (context->bound_addrs == NULL)
    {
        if (config->default_context != NULL)
            FatalError("Frag3 => only one non-bound engine can be specified.\n");

        config->default_context = context;
    }

    /* Now add this context to the internal list */
    if (config->frag3ContextList == NULL)
    {
        config->numFrag3Contexts = 1;
        config->frag3ContextList =
            (Frag3Context **)SnortPreprocAlloc(1, sizeof (Frag3Context *),
                             PP_FRAG3, PP_MEM_CATEGORY_CONFIG);
    }
    else
    {
        Frag3Context **tmpContextList;

        config->numFrag3Contexts++;
        tmpContextList = (Frag3Context **)
            SnortPreprocAlloc(config->numFrag3Contexts, sizeof (Frag3Context *),
                            PP_FRAG3, PP_MEM_CATEGORY_CONFIG);

        memcpy(tmpContextList, config->frag3ContextList,
               sizeof(Frag3Context *) * (config->numFrag3Contexts - 1));

        SnortPreprocFree(config->frag3ContextList,
                 (config->numFrag3Contexts-1) * sizeof (Frag3Context *), PP_FRAG3, 
                 PP_MEM_CATEGORY_CONFIG);
        config->frag3ContextList = tmpContextList;
    }

    config->frag3ContextList[config->numFrag3Contexts - 1] = context;

    /* print this engine config */
    Frag3PrintEngineConfig(context);
}

static int FragPolicyIdFromName(char *name)
{
    if (!name)
    {
        return FRAG_POLICY_DEFAULT;
    }

    if(!strcasecmp(name, "bsd"))
    {
        return FRAG_POLICY_BSD;
    }
    else if(!strcasecmp(name, "bsd-right"))
    {
        return FRAG_POLICY_BSD_RIGHT;
    }
    else if(!strcasecmp(name, "linux"))
    {
        return FRAG_POLICY_LINUX;
    }
    else if(!strcasecmp(name, "first"))
    {
        return FRAG_POLICY_FIRST;
    }
    else if(!strcasecmp(name, "windows"))
    {
        return FRAG_POLICY_WINDOWS;
    }
    else if(!strcasecmp(name, "solaris"))
    {
        return FRAG_POLICY_SOLARIS;
    }
    else if(!strcasecmp(name, "last"))
    {
        return FRAG_POLICY_LAST;
    }
    return FRAG_POLICY_DEFAULT;
}

#ifdef TARGET_BASED
int FragPolicyIdFromHostAttributeEntry(HostAttributeEntry *host_entry)
{
    if (!host_entry)
        return 0;

    host_entry->hostInfo.fragPolicy = FragPolicyIdFromName(host_entry->hostInfo.fragPolicyName);
    host_entry->hostInfo.fragPolicySet = 1;

    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
        "Frag3 INIT: %s(%d) for Entry %s\n",
        frag_policy_names[host_entry->hostInfo.fragPolicy],
        host_entry->hostInfo.fragPolicy,
        host_entry->hostInfo.fragPolicyName););

    return 0;
}
#endif

/**
 * Verify frag3 setup is complete
 *
 * @param args list of configuration arguments
 *
 * @return none
 */
static int Frag3VerifyConfigPolicy(
        struct _SnortConfig *sc,
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    Frag3Config *pPolicyConfig = (Frag3Config *)pData;

    if ( pPolicyConfig->disabled )
        return 0;

    //do any housekeeping before processingFrag3Config
    if ((policyId != getDefaultPolicy())
        && (pPolicyConfig->numFrag3Contexts == 0))
    {
        WarningMessage("Frag3VerifyConfig: PolicyId %d, policy engine required "
                "but not configured.\n", policyId);
        return -1;
    }

#ifdef TARGET_BASED
    SFAT_SetPolicyIds(FragPolicyIdFromHostAttributeEntry, policyId);
#endif

    return 0;
}

static int Frag3VerifyConfig(struct _SnortConfig *sc)
{
    if (sfPolicyUserDataIterate (sc, frag3_config, Frag3VerifyConfigPolicy))
        return -1;

    return 0;
}

/**
 * Handle the preallocation of frags
 *
 * @param int unused
 *        void *arg unused inputs
 *        (these aren't used, just need to match function prototype)
 *
 * @return none
 */
static void Frag3PostConfigInit(struct _SnortConfig *sc, void *arg)
{
    Frag3Frag *tmp; /* for initializing the prealloc queue */
    unsigned int i;          /* counter */
    Frag3Config *config = NULL;

    config = sfPolicyUserDataGetDefault(frag3_config);
    if (config == NULL)
        return;

    pkt_snaplen = DAQ_GetSnapLen();

    /*
     * user has decided to prealloc the node structs for performance
     */
    if(config->use_prealloc)
    {
        if (config->static_frags == 0)
        {
            config->static_frags = (uint32_t)(config->memcap /
                (sizeof(Frag3Frag) + sizeof(uint8_t) * pkt_snaplen) + 1);

            config->ten_percent = config->static_frags >> 5;
        }

        for (i = 0; i < config->static_frags; i++)
        {
            tmp = (Frag3Frag *) SnortPreprocAlloc(1, sizeof(Frag3Frag),
                                   PP_FRAG3, PP_MEM_CATEGORY_CONFIG);
            tmp->fptr = (uint8_t *) SnortPreprocAlloc(pkt_snaplen, sizeof(uint8_t), 
                                   PP_FRAG3, PP_MEM_CATEGORY_CONFIG);
            Frag3PreallocPush(tmp);
        }

        prealloc_nodes_in_use = 0;
    }
}

/**
 * Config parser for global config.
 *
 * @param args List of configuration parameters
 *
 * @return none
 */
static void Frag3ParseGlobalArgs(Frag3Config *gconfig, char *args)
{
    char **toks;
    int num_toks;
    int i = 0;
    char *index;
    char **stoks = NULL;
    int s_toks;
    char *endPtr;
    long ivalue;
    unsigned long value;

    if ((args == NULL) || (gconfig == NULL))
        return;

    toks = mSplit(args, ",", 0, &num_toks, 0);
    for (i = 0; i < num_toks; i++)
    {
        index = toks[i];

        stoks = mSplit(index, " ", 0, &s_toks, 0);

        if(!strcasecmp(stoks[0], "max_frags"))
        {
            if (s_toks != 2)
            {
                FatalError("%s(%d) => Missing argument to max_frags in "
                           "config file.\n",
                           file_name, file_line);
            }

            gconfig->max_frags = ivalue = strtol(stoks[1], &endPtr, 10);

            if ((endPtr == &stoks[1][0]) || (ivalue <= 0))
            {
                FatalError("%s(%d) => Invalid max_frags in config file. "
                           "Integer parameter required.\n", file_name,
                           file_line);
            }
        }
        else if(!strcasecmp(stoks[0], "memcap"))
        {
            if (s_toks != 2)
            {
                FatalError("%s(%d) => Missing argument to memcap in "
                           "config file.\n",
                           file_name, file_line);
            }

            gconfig->memcap = value = strtoul(stoks[1], &endPtr, 10);

            if (!*stoks[1] || *stoks[1] == '-' || *endPtr)
            {
                FatalError("%s(%d) => Invalid memcap in config file. "
                           "Integer parameter required.\n", file_name,
                           file_line);
            }

            if (gconfig->memcap < MIN_FRAG_MEMCAP)
            {
                LogMessage("WARNING: %s(%d) => Ludicrous (<16k) memcap "
                           "size, setting to default (%d bytes)\n",
                           file_name, file_line, FRAG_MEMCAP);

                gconfig->memcap = FRAG_MEMCAP;
            }

            /* ok ok, it's really 9.375%, sue me */
            gconfig->ten_percent = ((gconfig->memcap >> 5) + (gconfig->memcap >> 6));
        }
        else if(!strcasecmp(stoks[0], "prealloc_memcap"))
        {
            /* Use memcap to calculate prealloc_frag value */
            unsigned long memcap = FRAG_MEMCAP;

            if (s_toks != 2)
            {
                FatalError("%s(%d) => Missing argument to prealloc_memcap in "
                           "config file.\n",
                           file_name, file_line);
            }

            memcap = value = strtoul(stoks[1], &endPtr, 10);

            if (!*stoks[1] || *stoks[1] == '-' || *endPtr)
            {
                FatalError("%s(%d) => Invalid prealloc_memcap in config file. "
                           "Integer parameter required.\n", file_name,
                           file_line);
            }

            if(memcap <MIN_FRAG_MEMCAP)
            {
                LogMessage("WARNING: %s(%d) => Ludicrous (<16k) prealloc_memcap "
                           "size, setting to default (%d bytes)\n",
                           file_name, file_line, FRAG_MEMCAP);
                memcap = FRAG_MEMCAP;
            }

            gconfig->use_prealloc = 1;
            gconfig->memcap = memcap;
        }
        else if(!strcasecmp(stoks[0], "prealloc_frags"))
        {
            if (s_toks != 2)
            {
                FatalError("%s(%d) => Missing argument to prealloc_frags "
                           "in config file.\n",
                           file_name, file_line);
            }

            gconfig->static_frags = value = strtoul(stoks[1], &endPtr, 10);
            gconfig->use_prealloc_frags = gconfig->use_prealloc = 1;

            if (!*stoks[1] || *stoks[1] == '-' || *endPtr || value > MAX_PREALLOC_FRAGS)
            {
                FatalError("%s(%d) => Invalid prealloc_frags in config file. Entered value is not allowed. "
                        "Integer parameter (in the range of 0 to %d) required.\n", file_name,
                        file_line,MAX_PREALLOC_FRAGS);
            }
        }
        else if(!strcasecmp(stoks[0], "disabled"))
        {
            gconfig->disabled = 1;
        }
        else
        {
            FatalError("%s(%d) => Invalid Frag3 global option (%s)\n",
                       file_name, file_line, index);
        }

        mSplitFree(&stoks, s_toks);
    }

    mSplitFree(&toks, num_toks);
}

/**
 * Config parser for engine context config.
 *
 * @param args List of configuration parameters
 *
 * @return none
 */
static void Frag3ParseArgs(struct _SnortConfig *sc, char *args, Frag3Context *context)
{
    char **toks;
    int num_toks;
    int i = 0;

    toks = mSplit(args, " ", 13, &num_toks, 0);

    while(i < num_toks)
    {
        int error = 0;
        int increment = 1;
        char *index = toks[i];
        char *arg = NULL;
        char *endptr;
        int32_t value = 0;

        /* In case an option takes an argument */
        if ((i + 1) < num_toks)
            arg = toks[i + 1];

        if(!strcasecmp(index, "timeout"))
        {
            if (arg == NULL)
            {
                error = 1;
            }
            else
            {
                value = SnortStrtol(arg, &endptr, 10);
                if ((errno == ERANGE) || (*endptr != '\0') || (value < 0))
                    error = 1;
            }

            if (error)
            {
                ParseError("Bad timeout in frag3 config.  Positive integer "
                        "parameter required.");
            }

            increment = 2;
            context->frag_timeout = (uint32_t)value;
        }
        else if(!strcasecmp(index, "min_ttl"))
        {
            if (arg == NULL)
            {
                error = 1;
            }
            else
            {
                value = SnortStrtol(arg, &endptr, 10);
                if ((errno == ERANGE) || (*endptr != '\0')
                        || (value < 0) || (value > UINT8_MAX))
                {
                    error = 1;
                }
            }

            if (error)
            {
                ParseError("Bad min_ttl in frag3 config.  Positive integer "
                        "less than 256 required.");
            }

            increment = 2;
            context->min_ttl = (uint8_t)value;
        }
        else if(!strcasecmp(index, "detect_anomalies"))
        {
            context->frag3_alerts |= FRAG3_DETECT_ANOMALIES;
        }
        else if(!strcasecmp(index, "policy"))
        {
            if (arg == NULL)
            {
                ParseError("Frag3 policy requires a policy "
                        "identifier argument.");
            }

            increment = 2;
            context->frag_policy = FragPolicyIdFromName(arg);

            if ((context->frag_policy == FRAG_POLICY_DEFAULT) &&
                (strcasecmp(arg, "bsd")))
            {
                ParseError("Bad policy name \"%s\" in frag3 config.", arg);
            }
        }
        else if(!strcasecmp(index, "bind_to"))
        {
            if (arg == NULL)
            {
                ParseError("Frag3 bind_to requires an IP list or "
                        "CIDR block argument.");
            }

            /* Fatals on bad ip address */
            context->bound_addrs = IpAddrSetParse(sc, arg);
            increment = 2;
        }
        else if(!strcasecmp(index, "min_fragment_length"))
        {
            if (arg == NULL)
            {
                error = 1;
            }
            else
            {
                value = SnortStrtol(arg, &endptr, 10);
                if ((errno == ERANGE) || (*endptr != '\0') || (value < 0))
                    error = 1;
            }

            if (error)
            {
                ParseError("Bad min_fragment_length in frag3 config.  Positive "
                        "integer parameter required.");
            }

            increment = 2;
            context->min_fragment_length = (uint32_t)value;
        }
        else if(!strcasecmp(index, "overlap_limit"))
        {
            if (arg == NULL)
            {
                error = 1;
            }
            else
            {
                value = SnortStrtol(arg, &endptr, 10);
                if ((errno == ERANGE) || (*endptr != '\0') || (value < 0))
                    error = 1;
            }

            if (error)
            {
                ParseError("Bad overlap_limit in frag3 config.  Positive "
                        "integer parameter required.");
            }

            increment = 2;
            context->overlap_limit = (uint32_t)value;
        }
        else
        {
            ParseError("Invalid Frag3 engine option (%s).", index);
        }

        i += increment;
    }

    mSplitFree(&toks, num_toks);
}

/**
 * Main runtime entry point for Frag3
 *
 * @param p Current packet to process.
 * @param context Context for this defrag engine
 *
 * @return none
 */
static void Frag3Defrag(Packet *p, void *context)
{
    FRAGKEY fkey;            /* fragkey for this packet */
    FragTracker *ft;         /* FragTracker to process the packet on */
    Frag3Context *f3context = NULL; /* engine context */
    int engineIndex;
    int insert_return = 0;   /* return value from the insert function */
    tSfPolicyId policy_id = getNapRuntimePolicy();
    PROFILE_VARS;

    // preconditions - what we registered for
    assert(IPH_IS_VALID(p) && !(p->error_flags & PKT_ERR_CKSUM_IP));

    /* check to make sure this preprocessor should run */
    if ( !p->frag_flag )
        return;

    frag3_eval_config = (Frag3Config *)sfPolicyUserDataGet(frag3_config, policy_id);

    memset(&fkey, 0, sizeof(FRAGKEY));
    ft = Frag3GetTracker(p, &fkey);
    if (ft != NULL)
    {
        f3context = ft->context;
        frag3_eval_config = (Frag3Config *)sfPolicyUserDataGet(ft->config, ft->policy_id);
    }

    if (frag3_eval_config == NULL)
        return;

    if (ft == NULL)
    {
        /* Find an engine context for this packet */
        for (engineIndex = 0; engineIndex < frag3_eval_config->numFrag3Contexts; engineIndex++)
        {
            f3context = frag3_eval_config->frag3ContextList[engineIndex];

            if (f3context->bound_addrs == NULL)
                continue;

            /* Does this engine context handle fragments to this IP address? */
            if(sfvar_ip_in(f3context->bound_addrs, GET_DST_ADDR(p)))
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                                            "[FRAG3] Found engine context in IpAddrSet\n"););
                    break;
                }
        }

        if (engineIndex == frag3_eval_config->numFrag3Contexts)
            f3context = frag3_eval_config->default_context;

        if (!f3context)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                                    "[FRAG3] Could not find Frag3 engine context "
                                    "for IP %s\n", inet_ntoa(GET_SRC_ADDR(p))););
            return;
        }
    }

    /*
     * First case: if frag offset is 0 & UDP, let that packet go
     * through the rest of the system.  Ugly HACK to detect DNS
     * attack on 0 offset UDP.
     *
     * Second case: If frag offset is 0 & !more frags, this is a
     * full-frame "fragment", let the packet go through the rest
     * of the system.
     *
     * In other words:
     *   a = frag_offset != 0
     *   b = !UDP
     *   c = More Fragments
     *
     * if (a | (b & c))
     *    Disable Inspection since we'll look at the payload in
     *    a rebuilt packet later.  So don't process it further.
     */
    if ((p->frag_offset != 0) || ((GET_IPH_PROTO(p) != IPPROTO_UDP) && (p->mf)))
    {
        DisableDetect( p );
        otn_tmp = NULL;
    }

    /*
     * pkt's not going to make it to the target, bail
     */
    if(GET_IPH_TTL(p) < f3context->min_ttl)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                "[FRAG3] Fragment discarded due to low TTL "
                "[0x%X->0x%X], TTL: %d  " "Offset: %d Length: %d\n",
                ntohl(p->iph->ip_src.s_addr),
                ntohl(p->iph->ip_dst.s_addr),
                GET_IPH_TTL(p), p->frag_offset,
                p->dsize););

        EventAnomScMinTTL(f3context);
        f3stats.discards++;
        return;
    }

    f3stats.total++;
    UpdateIPFragStats(&sfBase, p->pkth->caplen);

    PREPROC_PROFILE_START(frag3PerfStats);

    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                "\n++++++++++++++++++++++++++++++++++++++++++++++\n"););
    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                "[**] [FRAG3] Inspecting fragment...\n"););
    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                "[FRAG3] Got frag packet (mem use: %ld frag "
                "trackers: %d  p->pkt_flags: 0x%X "
                "prealloc nodes in use: %lu/%lu)\n",
                frag3_mem_in_use,
                sfxhash_count(f_cache),
                p->packet_flags, prealloc_nodes_in_use,
                frag3_eval_config->static_frags););

    pkttime = (struct timeval *) &p->pkth->ts;

    /*
     * try to get the tracker that this frag should go with
     */
    if (ft == NULL)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FRAG, "Adding New FragTracker...\n"););

        /*
         * first frag for this packet, start a new tracker
         */
        Frag3NewTracker(p, &fkey, f3context);

        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                    "[FRAG3] mem use: %ld frag "
                    "trackers: %d  prealloc "
                    "nodes in use: %lu/%lu\n",
                    frag3_mem_in_use,
                    sfxhash_count(f_cache),
                    prealloc_nodes_in_use,
                    frag3_eval_config->static_frags););
        /*
         * all done, return control to Snort
         */
        PREPROC_PROFILE_END(frag3PerfStats);
        return;
    }
    else if (Frag3Expire(p, ft, f3context) == FRAG_TRACKER_TIMEOUT)
    {
        /* Time'd out FragTrackers are just purged of their packets.
         * Reset the timestamp per this packet.
         * And reset the rest of the tracker as if this is the
         * first packet on the tracker, and continue. */

        /* This fixes an issue raised on bugtraq relating to
         * timeout frags not getting purged correctly when
         * the entire set of frags show up later. */

        ft->ttl = GET_IPH_TTL(p); /* store the first ttl we got */
        ft->calculated_size = 0;
        ft->alerted = 0;
        ft->frag_flags = 0;
        ft->frag_bytes = 0;
        ft->frag_pkts = 0;
        ft->alert_count = 0;
        ft->ip_options_len = 0;
        ft->ip_option_count = 0;
        ft->ip_options_data = NULL;
        ft->copied_ip_options_len = 0;
        ft->copied_ip_option_count = 0;
        ft->context = f3context;
        ft->ordinal = 0;
    }

    // Update frag time when we get a frag associated with this tracker
    ft->frag_time.tv_sec = p->pkth->ts.tv_sec;
    ft->frag_time.tv_usec = p->pkth->ts.tv_usec;

    DEBUG_WRAP(DebugMessage(DEBUG_FRAG, "Found frag tracker\n"););

    //dont forward fragments to target if some previous fragment was dropped
    if ( ft->frag_flags & FRAG_DROP_FRAGMENTS )
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                    "Blocking fragments due to earlier fragment drop\n"););
        DisableDetect( p );
        Active_DAQDropPacket( p );
        if (pkt_trace_enabled)
        {
            addPktTraceData(VERDICT_REASON_DEFRAG, snprintf(trace_line, MAX_TRACE_LINE,
                "Defragmentation: earlier fragment was already blocked, %s\n", getPktTraceActMsg()));
        }
        else addPktTraceData(VERDICT_REASON_DEFRAG, 0);
        f3stats.drops++;
    }

    /*
     * insert the fragment into the FragTracker
     */
    if((insert_return = Frag3Insert(p, ft, &fkey, f3context)) != FRAG_INSERT_OK)
    {
        /*
         * we can pad this switch out for a variety of entertaining behaviors
         * later if we're so inclined
         */
        switch(insert_return)
        {
            case FRAG_INSERT_FAILED:
#ifdef DEBUG
                LogMessage("WARNING: Insert into Fraglist failed, "
                           "(offset: %u).\n", p->frag_offset);
#endif
                PREPROC_PROFILE_END(frag3PerfStats);
                return;
            case FRAG_INSERT_TTL:
                DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                        "[FRAG3] Fragment discarded due to large TTL Delta "
                        "[0x%X->0x%X], TTL: %d  orig TTL: %d "
                        "Offset: %d Length: %d\n",
                        ntohl(p->iph->ip_src.s_addr),
                        ntohl(p->iph->ip_dst.s_addr),
                        GET_IPH_TTL(p), ft->ttl, p->frag_offset,
                        p->dsize););
                f3stats.discards++;
                PREPROC_PROFILE_END(frag3PerfStats);
                return;
            case FRAG_INSERT_ATTACK:
            case FRAG_INSERT_ANOMALY:
                f3stats.discards++;
                PREPROC_PROFILE_END(frag3PerfStats);
                return;
            case FRAG_INSERT_TIMEOUT:
#ifdef DEBUG
                LogMessage("WARNING: Insert into Fraglist failed due to timeout, "
                           "(offset: %u).\n", p->frag_offset);
#endif
                PREPROC_PROFILE_END(frag3PerfStats);
                return;
            case FRAG_INSERT_OVERLAP_LIMIT:
#ifdef DEBUG
                LogMessage("WARNING: Excessive IP fragment overlap, "
                           "(More: %u, offset: %u, offsetSize: %u).\n",
                           p->mf, (p->frag_offset<<3), p->ip_frag_len);
#endif
                f3stats.discards++;
                PREPROC_PROFILE_END(frag3PerfStats);
                return;
            default:
                break;
        }
    }

    p->fragtracker = (void *)ft;

    /*
     * check to see if it's reassembly time
     */
    if(Frag3IsComplete(ft))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                    "[*] Fragment is complete, rebuilding!\n"););

        /*
         * if the frag completes but it's bad we're just going to drop it
         * instead of wasting time on putting it back together
         */
        if(!(ft->frag_flags & FRAG_BAD))
        {
            Frag3Rebuild(ft, p);

            if (p->frag_offset != 0 ||
                (GET_IPH_PROTO(p) != IPPROTO_UDP && ft->frag_flags & FRAG_REBUILT))
            {
                /* Need to reset some things here because the
                 * rebuilt packet will have reset the do_detect
                 * flag when it hits Preprocess.
                 */
                do_detect_content = do_detect = 0;
                otn_tmp = NULL;
            }
        }

        if (Active_PacketWasDropped())
        {
            Frag3DeleteTracker(ft);
            ft->frag_flags |= FRAG_DROP_FRAGMENTS;
        }
        else
        {
            Frag3RemoveTracker(&fkey, ft);
            p->fragtracker = NULL;

            DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                        "[FRAG3] Dumped fragtracker (mem use: %ld frag "
                        "trackers: %d  prealloc nodes in use: %lu/%lu)\n",
                        frag3_mem_in_use, sfxhash_count(f_cache),
                        prealloc_nodes_in_use, frag3_eval_config->static_frags););
        }
    }

    PREPROC_PROFILE_END(frag3PerfStats);
    return;
}

/**
 * Check to see if a FragTracker has timed out
 *
 * @param current_time Time at this moment
 * @param start_time Time to compare current_time to
 * @param f3context Engine context
 *
 * @return status
 * @retval  FRAG_TIMEOUT Current time diff is greater than the current
 *                       context's timeout value
 * @retval  FRAG_TIME_OK Current time diff is within the context's prune
 *                       window
 */
static inline int CheckTimeout(struct timeval *current_time,
        struct timeval *start_time,
        Frag3Context *f3context)
{
    struct timeval tv_diff; /* storage struct for the difference between
                               current_time and start_time */

    TIMERSUB(current_time, start_time, &tv_diff);
    if(tv_diff.tv_sec >= (int)f3context->frag_timeout)
    {
        return FRAG_TIMEOUT;
    }

    return FRAG_TIME_OK;
}

/**
 * Time-related expiration of fragments from the system.  Checks the current
 * FragTracker for timeout, then walks up the LRU list looking to see if
 * anyone should have timed out.
 *
 * @param p Current packet (contains pointer to the current timestamp)
 * @param ft FragTracker to check for a timeout
 * @param fkey FragKey of the current FragTracker for sfxhash lookup
 * @param f3context Context of the defrag engine, contains the timeout value
 *
 * @return status
 * @retval FRAG_TRACKER_TIMEOUT The current FragTracker has timed out
 * @retval FRAG_OK The current FragTracker has not timed out
 */
static inline int Frag3Expire(Packet *p, FragTracker *ft, Frag3Context *f3context)
{
    /*
     * Check the FragTracker that was passed in first
     */
    if(CheckTimeout(
                pkttime,
                &(ft)->frag_time,
                f3context) == FRAG_TIMEOUT)
    {
        /*
         * Oops, we've timed out, whack the FragTracker
         */
#if defined(DEBUG_FRAG3) && defined(DEBUG)
        if (DEBUG_FRAG & GetDebugLevel())
        {
            char *src_str = SnortStrdup(FragIPToStr(ft->sip, ft->ipver));
            LogMessage("(spp_frag3) Current Fragment dropped due to timeout! "
                "[%s->%s ID: %d]\n", src_str, FragIPToStr(ft->dip, ft->ipver), ft->id);
            free(src_str);
        }
#endif

        /*
         * Don't remove the tracker.
         * Remove all of the packets that are stored therein.
         *
         * If the existing tracker times out because of a delay
         * relative to the timeout
         */
        //Frag3RemoveTracker(fkey, ft);
        Frag3DeleteTracker(ft);

        f3stats.timeouts++;
        sfBase.iFragTimeouts++;

        return FRAG_TRACKER_TIMEOUT;
    }

    return FRAG_OK;
}

/**
 * Check to see if we've got the first or last fragment on a FragTracker and
 * set the appropriate frag_flags
 *
 * @param p Packet to get the info from
 * @param ft FragTracker to set the flags on
 *
 * @return none
 */
static inline int Frag3CheckFirstLast(Packet *p, FragTracker *ft)
{
    uint16_t fragLength;
    int retVal = FRAG_FIRSTLAST_OK;
    uint16_t endOfThisFrag;

    /* set the frag flag if this is the first fragment */
    if(p->mf && p->frag_offset == 0)
    {
        ft->frag_flags |= FRAG_GOT_FIRST;

        DEBUG_WRAP(DebugMessage(DEBUG_FRAG, "Got first frag\n"););
    }
    else if((!p->mf) && (p->frag_offset > 0)) /* set for last frag too */
    {
        /* Use the actual length here, because packet may have been
        * truncated.  Don't want to try to copy more than we actually
        * captured. */
        //fragLength = p->actual_ip_len - GET_IPH_HLEN(p) * 4;
        fragLength = p->ip_frag_len;
        endOfThisFrag = (p->frag_offset << 3) + fragLength;

        if (ft->frag_flags & FRAG_GOT_LAST)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_FRAG, "Got last frag again!\n"););
            switch (ft->frag_policy)
            {
                case FRAG_POLICY_BSD:
                case FRAG_POLICY_LINUX:
                case FRAG_POLICY_BSD_RIGHT:
                case FRAG_POLICY_LAST:
                case FRAG_POLICY_WINDOWS:
                case FRAG_POLICY_FIRST:
                    if (ft->calculated_size > endOfThisFrag)
                    {
                       /* Already have a 'last frag' with a higher
                        * end point.  Leave it as is.
                        *
                        * Some OS's do not respond at all -- we'll
                        * still try to rebuild anyway in that case,
                        * because there is really something wrong
                        * and we should look at it.
                        */
                        retVal = FRAG_LAST_DUPLICATE;
                    }
                    break;
                case FRAG_POLICY_SOLARIS:
                    if (ft->calculated_size > endOfThisFrag)
                    {
                       /* Already have a 'last frag' with a higher
                        * end point.  Leave it as is.
                        *
                        * Some OS's do not respond at all -- we'll
                        * still try to rebuild anyway in that case,
                        * because there is really something wrong
                        * and we should look at it.
                        */
                        retVal = FRAG_LAST_DUPLICATE;
                    }
                    else
                    {
                        /* Solaris does some weird stuff here... */
                        /* Usually, Solaris takes the higher end point.
                         * But in one strange case (when it hasn't seen
                         * any frags beyond the existing last frag), it
                         * actually appends that new last frag to the
                         * end of the previous last frag, regardless of
                         * the offset.  Effectively, it adjusts the
                         * offset of the new last frag to immediately
                         * after the existing last frag.
                         */
                        /* XXX: how to handle that case? punt?  */
                        retVal = FRAG_LAST_OFFSET_ADJUST;
                    }
                    break;
            }
        }

        ft->frag_flags |= FRAG_GOT_LAST;

        /*
         * If this is the last frag (and we don't have a frag that already
         * extends beyond this one), set the size that we're expecting.
         */
        if ((ft->calculated_size < endOfThisFrag) &&
            (retVal != FRAG_LAST_OFFSET_ADJUST))
        {
            ft->calculated_size = endOfThisFrag;

            DEBUG_WRAP(DebugMessage(DEBUG_FRAG, "Got last frag, Bytes: %d, "
                    "Calculated size: %d\n",
                    ft->frag_bytes,
                    ft->calculated_size););
        }
    }

    if (p->frag_offset != 0)
    {
        ft->frag_flags |= FRAG_NO_BSD_VULN;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_FRAG, "Frag Status: %s:%s\n",
                ft->frag_flags&FRAG_GOT_FIRST?"FIRST":"No FIRST",
                ft->frag_flags&FRAG_GOT_LAST?"LAST":"No LAST"););
    return retVal;
}

/**
 * Lookup a FragTracker in the f_cache sfxhash table based on an input key
 *
 * @param p The current packet to get the key info from
 * @param fkey Pointer to a container for the FragKey
 *
 * @return Pointer to the FragTracker in the hash bucket or NULL if there is
 *         no fragment in the hash bucket
 */
static FragTracker *Frag3GetTracker(Packet *p, FRAGKEY *fkey)
{
    FragTracker *returned; /* FragTracker ptr returned by the lookup */

    /*
     * we have to setup the key first, downstream functions depend on
     * it being setup here
     */
    if (IS_IP4(p))
    {
        COPY4(fkey->sip, sfaddr_get_ip6_ptr(&p->ip4h->ip_addrs->ip_src));
        COPY4(fkey->dip, sfaddr_get_ip6_ptr(&p->ip4h->ip_addrs->ip_dst));
        fkey->id = GET_IPH_ID(p);
        fkey->ipver = 4;
        fkey->proto = GET_IPH_PROTO(p);
    }
    else
    {
        IP6Frag *fragHdr;
        COPY4(fkey->sip, sfaddr_get_ip6_ptr(&p->ip6h->ip_addrs->ip_src));
        COPY4(fkey->dip, sfaddr_get_ip6_ptr(&p->ip6h->ip_addrs->ip_dst));
        fkey->ipver = 6;
        /* Data points to the offset, and does not include the next hdr
         * and reserved.  Offset it by -2 to get there */
        fragHdr = (IP6Frag *)p->ip6_extensions[p->ip6_frag_index].data;
        /* Can't rely on the next header.  Only the 0 offset packet
         * is required to have it in the frag header */
        //fkey->proto = fragHdr->ip6f_nxt;
        fkey->proto = 0;
        fkey->id = fragHdr->ip6f_ident;
    }
    if (p->vh && !ScVlanAgnostic())
        fkey->vlan_tag = (uint16_t)VTH_VLAN(p->vh);
    else
        fkey->vlan_tag = 0;

#ifdef MPLS
    if(ScMplsOverlappingIp() && p->mpls)
        fkey->mlabel = p->mplsHdr.label;
    else
        fkey->mlabel = 0;
#endif

#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
    fkey->address_space_id_dst = DAQ_GetDestinationAddressSpaceID(p->pkth); 
    fkey->address_space_id_src = DAQ_GetSourceAddressSpaceID(p->pkth); 
#else
    if (!ScAddressSpaceAgnostic())
        fkey->addressSpaceId = DAQ_GetAddressSpaceID(p->pkth);
    else
        fkey->addressSpaceId = 0;
#endif
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
    fkey->carrierId = GET_OUTER_IPH_PROTOID(p, pkth);
#endif

    /*
     * if the hash table is empty we're done
     */
    if(sfxhash_count(f_cache) == 0)
        return NULL;

    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                "[*] Looking up FragTracker using key:\n"););

#ifdef DEBUG_FRAG3
    PrintFragKey(fkey);
#endif

    returned = (FragTracker *) sfxhash_find(f_cache, fkey);

    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                "Frag3GetTracker returning %p for\n", returned););

    return returned;
}

/**
 * Handle IP Options in fragmented packets.
 *
 * @param ft Current frag tracker for this packet
 * @param p Current packet to check for options
 * @param context In case we get an anomaly
 *
 * @return status
 * @retval 0 on an error
 * @retval 1 on success
 */
static int Frag3HandleIPOptions(FragTracker *ft,
                                Packet *p)
{
    unsigned int i = 0;          /* counter */
    if(p->frag_offset == 0)
    {
        /*
         * This is the first packet.  If it has IP options,
         * save them off, so we can set them on the reassembled packet.
         */
        if (p->ip_options_len)
        {
            if (ft->ip_options_data)
            {
                /* Already seen 0 offset packet and copied some IP options */
                if ((ft->frag_flags & FRAG_GOT_FIRST)
                        && (ft->ip_option_count != p->ip_option_count))
                {
                    EventAnomIpOpts(ft->context);
                }
            }
            else
            {
                /* Allocate and copy in the options */
                ft->ip_options_data = SnortPreprocAlloc(1, p->ip_options_len, PP_FRAG3,
                                                         PP_MEM_CATEGORY_SESSION);
                memcpy(ft->ip_options_data, p->ip_options_data, p->ip_options_len);
                ft->ip_options_len = p->ip_options_len;
                ft->ip_option_count = p->ip_option_count;
            }
        }
    }
    else
    {
        /* check that options match those from other non-offset 0 packets */

        /* XXX: could check each individual option here, but that
         * would be performance ugly.  So, we'll just check that the
         * option counts match.  Alert if invalid, but still include in
         * reassembly.
         */
        if (ft->copied_ip_option_count)
        {
            if (ft->copied_ip_option_count != p->ip_option_count)
            {
                EventAnomIpOpts(ft->context);
            }
        }
        else
        {
            ft->copied_ip_option_count = p->ip_option_count;
            for (i = 0;i< p->ip_option_count && i < IP_OPTMAX; i++)
            {
                /* Is the high bit set?  If not, weird anomaly. */
                if (!(p->ip_options[i].code & 0x80))
                    EventAnomIpOpts(ft->context);
            }
        }
    }
    return 1;
}

int FragGetPolicy(Packet *p, Frag3Context *f3context)
{
#ifdef TARGET_BASED
    int frag_policy;
    /* Not caching this host_entry in the frag tracker so we can
     * swap the table out after processing this packet if we need
     * to.  */
    HostAttributeEntry *host_entry;

    if (!IsAdaptiveConfigured())
        return f3context->frag_policy;

    host_entry = SFAT_LookupHostEntryByDst(p);

    if (host_entry && (isFragPolicySet(host_entry) == POLICY_SET))
    {
        frag_policy = getFragPolicy(host_entry);

        if (frag_policy != SFAT_UNKNOWN_FRAG_POLICY)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                "FragGetPolicy: Policy Map Entry: %d(%s)\n",
                frag_policy, frag_policy_names[frag_policy]););

            return frag_policy;
        }
    }
#endif

    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
        "FragGetPolicy: Using configured default %d(%s)\n",
        f3context->frag_policy, frag_policy_names[f3context->frag_policy]););

    return f3context->frag_policy;
}

/**
 * Didn't find a FragTracker in the hash table, create a new one and put it
 * into the f_cache
 *
 * @param p Current packet to fill in FragTracker fields
 * @param fkey FragKey struct to use for table insertion
 *
 * @return status
 * @retval 0 on an error
 * @retval 1 on success
 */
static int Frag3NewTracker(Packet *p, FRAGKEY *fkey, Frag3Context *f3context)
{
    FragTracker *tmp;
    Frag3Frag *f = NULL;
    //int ret = 0;
    const uint8_t *fragStart;
    uint16_t fragLength;
    uint16_t frag_end;
    SFXHASH_NODE *hnode;
    tSfPolicyId policy_id = getNapRuntimePolicy();

    fragStart = p->ip_frag_start;
    //fragStart = (uint8_t *)p->iph + GET_IPH_HLEN(p) * 4;
    /* Use the actual length here, because packet may have been
     * truncated.  Don't want to try to copy more than we actually
     * captured. */
    //fragLength = p->actual_ip_len - GET_IPH_HLEN(p) * 4;
    fragLength = p->ip_frag_len;
#ifdef DEBUG_MSGS
    if (p->actual_ip_len != ntohs(GET_IPH_LEN(p)))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
            "IP Actual Length (%d) != specified length (%d), "
            "truncated packet (%d)?\n",
            p->actual_ip_len, ntohs(GET_IPH_LEN(p)), pkt_snaplen););
    }
#endif

    /* Just to double check */
    if (fragLength > pkt_snaplen)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
            "Overly large fragment %d 0x%x 0x%x %d\n",
            fragLength, GET_IPH_LEN(p), GET_IPH_OFF(p),
            p->frag_offset << 3););

        /* Ah, crap.  Return that tracker. */
        return 0;
    }

    // Try to get a new one
    if (!(hnode = sfxhash_get_node(f_cache, fkey)) || !hnode->data)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                    "Frag3NewTracker: sfxhash_get_node() failed\n"););
        return 0;
    }

    tmp = (FragTracker *)hnode->data;
    memset(tmp, 0, sizeof(FragTracker));

    /*
     * setup the frag tracker
     */
    COPY4(tmp->sip,fkey->sip);
    COPY4(tmp->dip,fkey->dip);
    tmp->id = fkey->id;
    if (IS_IP4(p))
    {
        tmp->protocol = fkey->proto;
        tmp->ipver = 4;
    }
    else /* IPv6 */
    {
        if (p->frag_offset == 0)
        {
            IP6Frag *fragHdr = (IP6Frag *)p->ip6_extensions[p->ip6_frag_index].data;
            tmp->protocol = fragHdr->ip6f_nxt;
        }
        tmp->ipver = 6;
    }
    tmp->ttl = GET_IPH_TTL(p); /* store the first ttl we got */
    tmp->calculated_size = 0;
    tmp->alerted = 0;
    tmp->frag_flags = 0;
    tmp->frag_bytes = 0;
    tmp->frag_pkts = 0;
    tmp->frag_time.tv_sec = p->pkth->ts.tv_sec;
    tmp->frag_time.tv_usec = p->pkth->ts.tv_usec;
    tmp->alert_count = 0;
    tmp->ip_options_len = 0;
    tmp->ip_option_count = 0;
    tmp->ip_options_data = NULL;
    tmp->copied_ip_options_len = 0;
    tmp->copied_ip_option_count = 0;
    tmp->ordinal = 0;
    tmp->frag_policy = FragGetPolicy(p, f3context);
    tmp->context = f3context;

    tmp->policy_id = policy_id;
    tmp->config = frag3_config;
    ((Frag3Config *)sfPolicyUserDataGet(tmp->config, tmp->policy_id))->ref_count++;

    /*
     * get our first fragment storage struct
     */
    if(!frag3_eval_config->use_prealloc)
    {
        if(frag3_mem_in_use > frag3_eval_config->memcap)
        {
            if (Frag3Prune(tmp) == 0)
            {
                DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                    "Frag3NewTracker: Pruning failed\n"););

                return 0;
            }
        }

        f = (Frag3Frag *) SnortPreprocAlloc(1, sizeof(Frag3Frag), PP_FRAG3,
                                 PP_MEM_CATEGORY_SESSION);
        frag3_mem_in_use += sizeof(Frag3Frag);

        f->fptr = (uint8_t *) SnortPreprocAlloc(1, fragLength, PP_FRAG3,
                                 PP_MEM_CATEGORY_SESSION);
        frag3_mem_in_use += fragLength;

        sfBase.frag3_mem_in_use = frag3_mem_in_use;
    }
    else
    {
        while((f = Frag3PreallocPop()) == NULL)
        {
            if (Frag3Prune(tmp) == 0)
            {
                DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                    "Frag3NewTracker: Pruning failed\n"););

                return 0;
            }
        }
    }

    f3stats.fragnodes_created++;
    sfBase.iFragCreates++;
    sfBase.iCurrentFrags++;
    if (sfBase.iCurrentFrags > sfBase.iMaxFrags)
        sfBase.iMaxFrags = sfBase.iCurrentFrags;

    /* initialize the fragment list */
    tmp->fraglist = NULL;

    /*
     * setup the Frag3Frag struct with the current packet's data
     */
    memcpy(f->fptr, fragStart, fragLength);

    f->size = f->flen = fragLength;
    f->offset = p->frag_offset << 3;
    frag_end = f->offset + fragLength;
    f->ord = tmp->ordinal++;
    f->data = f->fptr;     /* ptr to adjusted start position */
    if (!p->mf)
    {
        f->last = 1;
    }
    else
    {
        /*
         * all non-last frags are supposed to end on 8-byte boundries
         */
        if(frag_end & 7)
        {
            /*
             * bonk/boink/jolt/etc attack...
             */
            DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                        "[..] Short frag (Bonk, etc) attack!\n"););

            EventAnomShortFrag(f3context);

            /* don't return, might still be interesting... */
        }

        /* can't have non-full fragments... */
        frag_end &= ~7;

        /* Adjust len to take into account the jolting/non-full fragment. */
        f->size = frag_end - f->offset;
    }

    /* insert the fragment into the frag list */
    tmp->fraglist = f;
    tmp->fraglist_tail = f;
    tmp->fraglist_count = 1;  /* XXX: Are these duplciates? */
    tmp->frag_pkts = 1;

    /*
     * mark the FragTracker if this is the first/last frag
     */
    Frag3CheckFirstLast(p, tmp);

    tmp->frag_bytes += fragLength;

    Frag3HandleIPOptions(tmp, p);

    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                "[#] accumulated bytes on FragTracker: %d\n",
                tmp->frag_bytes););

    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                "Initial fragment for tracker, ptr %p, offset %d, "
                "size %d\n", f, f->offset, f->size););

#ifdef DEBUG_FRAG3
    PrintFragKey(fkey);
#endif

    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                "Calling sfxhash(add), overhead at %lu\n",
                f_cache->overhead_bytes););

    f3stats.fragtrackers_created++;
    pc.frag_trackers++;

    p->fragtracker = (void *)tmp;

    return 1;
}

/**
 * Handle the creation of the new frag node and list insertion.
 * Separating this from actually calculating the values.
 *
 * @param ft FragTracker to hold the packet
 * @param fragStart Pointer to start of the packet data
 * @param fragLength Length of packet data
 * @param len Length of this fragment
 * @param slide Adjustment to make to left side of data (for left overlaps)
 * @param trunc Adjustment to maek to right side of data (for right overlaps)
 * @param frag_offset Offset for this fragment
 * @prarm left FragNode prior to this one
 * @param retFrag this one after its inserted (returned)
 *
 * @return status
 * @retval FRAG_INSERT_FAILED Memory problem, insertion failed
 * @retval FRAG_INSERT_OK All okay
 */
static int AddFragNode(FragTracker *ft,
                Packet *p,
                Frag3Context *f3context,
                const uint8_t *fragStart,
                int16_t fragLength,
                char lastfrag,
                int16_t len,
                uint16_t slide,
                uint16_t trunc,
                uint16_t frag_offset,
                Frag3Frag *left,
                Frag3Frag **retFrag)
{
    Frag3Frag *newfrag = NULL;  /* new frag container */
    int16_t newSize = len - slide - trunc;

    if (newSize <= 0)
    {
        /*
         * zero size frag
         */
        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
            "zero size frag after left & right trimming "
            "(len: %d  slide: %d  trunc: %d)\n",
            len, slide, trunc););

        f3stats.discards++;

#ifdef DEBUG_MSGS
        newfrag = ft->fraglist;
        while (newfrag)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                   "Size: %d, offset: %d, len %d, "
                   "Prev: 0x%x, Next: 0x%x, This: 0x%x, Ord: %d, %s\n",
                   newfrag->size, newfrag->offset,
                   newfrag->flen, newfrag->prev,
                   newfrag->next, newfrag, newfrag->ord,
                   newfrag->last ? "Last":""););
            newfrag = newfrag->next;
        }
#endif

        return FRAG_INSERT_ANOMALY;
    }

    /*
     * grab/generate a new frag node
     */
    if(!frag3_eval_config->use_prealloc)
    {
        if(frag3_mem_in_use > frag3_eval_config->memcap)
        {
            if (Frag3Prune(ft) == 0)
            {
                DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                    "Frag3Insert: Pruning failed\n"););

                return FRAG_INSERT_FAILED;
            }
        }

        /*
         * build a frag struct to track this particular fragment
         */
        newfrag = (Frag3Frag *) SnortPreprocAlloc(1, sizeof(Frag3Frag),
                                     PP_FRAG3, PP_MEM_CATEGORY_SESSION);
        frag3_mem_in_use += sizeof(Frag3Frag);

        /*
         * allocate some space to hold the actual data
         */
        newfrag->fptr = (uint8_t*)SnortPreprocAlloc(1, fragLength, PP_FRAG3,
                                                PP_MEM_CATEGORY_SESSION);
        frag3_mem_in_use += fragLength;

        sfBase.frag3_mem_in_use = frag3_mem_in_use;
    }
    else
    {
        /*
         * fragments are preallocated, grab one from the list
         */
        while((newfrag = Frag3PreallocPop()) == NULL)
        {
            if (Frag3Prune(ft) == 0)
            {
                DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                    "Frag3Insert: Pruning failed\n"););

                return FRAG_INSERT_FAILED;
            }
        }

        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                    "got newfrag (%p) from prealloc\n", newfrag););
    }

    f3stats.fragnodes_created++;

    newfrag->flen = fragLength;
    memcpy(newfrag->fptr, fragStart, fragLength);
    newfrag->ord = ft->ordinal++;

    /*
     * twiddle the frag values for overlaps
     */
    newfrag->data = newfrag->fptr + slide;
    newfrag->size = newSize;
    newfrag->offset = frag_offset;
    newfrag->last = lastfrag;

    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                "[+] Adding new frag, offset %d, size %d\n"
                "   nf->data = nf->fptr(%p) + slide (%d)\n"
                "   nf->size = len(%d) - slide(%d) - trunc(%d)\n",
                newfrag->offset, newfrag->size, newfrag->fptr,
                slide, fragLength, slide, trunc););

    /*
     * insert the new frag into the list
     */
    Frag3FraglistAddNode(ft, left, newfrag);

    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                "[*] Inserted new frag %d@%d ptr %p data %p prv %p nxt %p\n",
                newfrag->size, newfrag->offset, newfrag, newfrag->data,
                newfrag->prev, newfrag->next););

    /*
     * record the current size of the data in the fraglist
     */
    ft->frag_bytes += newfrag->size;

    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                "[#] accumulated bytes on FragTracker %d, count"
                " %d\n", ft->frag_bytes, ft->fraglist_count););

    *retFrag = newfrag;
    return FRAG_INSERT_OK;
}

/**
 * Duplicate a frag node and insert it into the list.
 *
 * @param ft FragTracker to hold the packet
 * @prarm left FragNode prior to this one (to be dup'd)
 * @param retFrag this one after its inserted (returned)
 *
 * @return status
 * @retval FRAG_INSERT_FAILED Memory problem, insertion failed
 * @retval FRAG_INSERT_OK All okay
 */
static int DupFragNode(FragTracker *ft,
                Frag3Frag *left,
                Frag3Frag **retFrag)
{
    Frag3Frag *newfrag = NULL;  /* new frag container */

    /*
     * grab/generate a new frag node
     */
    if(!frag3_eval_config->use_prealloc)
    {
        if(frag3_mem_in_use > frag3_eval_config->memcap)
        {
            if (Frag3Prune(ft) == 0)
            {
                DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                    "Frag3Insert: Pruning failed\n"););

                return FRAG_INSERT_FAILED;
            }
        }

        /*
         * build a frag struct to track this particular fragment
         */
        newfrag = (Frag3Frag *) SnortPreprocAlloc(1, sizeof(Frag3Frag),
                                    PP_FRAG3, PP_MEM_CATEGORY_SESSION);
        frag3_mem_in_use += sizeof(Frag3Frag);

        /*
         * allocate some space to hold the actual data
         */
        newfrag->fptr = (uint8_t*)SnortPreprocAlloc(1, left->flen, 
                                PP_FRAG3, PP_MEM_CATEGORY_SESSION);
        frag3_mem_in_use += left->flen;

        sfBase.frag3_mem_in_use = frag3_mem_in_use;
    }
    else
    {
        /*
         * fragments are preallocated, grab one from the list
         */
        while((newfrag = Frag3PreallocPop()) == NULL)
        {
            if (Frag3Prune(ft) == 0)
            {
                DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                    "Frag3Insert: Pruning failed\n"););

                return FRAG_INSERT_FAILED;
            }
        }

        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                    "got newfrag (%p) from prealloc\n", newfrag););
    }

    f3stats.fragnodes_created++;

    newfrag->ord = ft->ordinal++;
    /*
     * twiddle the frag values for overlaps
     */
    newfrag->flen = left->flen;
    memcpy(newfrag->fptr, left->fptr, newfrag->flen);
    newfrag->data = newfrag->fptr + (left->data - left->fptr);
    newfrag->size = left->size;
    newfrag->offset = left->offset;
    newfrag->last = left->last;

    /*
     * insert the new frag into the list
     */
    Frag3FraglistAddNode(ft, left, newfrag);

    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                "[*] Inserted new frag %d@%d ptr %p data %p prv %p nxt %p\n",
                newfrag->size, newfrag->offset, newfrag, newfrag->data,
                newfrag->prev, newfrag->next););

    /*
     * record the current size of the data in the fraglist
     */
    ft->frag_bytes += newfrag->size;

    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                "[#] accumulated bytes on FragTracker %d, count"
                " %d\n", ft->frag_bytes, ft->fraglist_count););

    *retFrag = newfrag;
    return FRAG_INSERT_OK;
}

/** checks for tiny fragments and raises appropriate alarm
 *
 * @param p Current packet to insert
 * @param ft FragTracker to hold the packet
 * @param fkey FragKey with the current FragTracker's key info
 * @param f3context context of the current engine for target-based defrag info
 *
 * @returns 1 if tiny fragment was detected, 0 otherwise
 */
static inline int checkTinyFragments(
        Frag3Context *f3context,
        Packet *p,
        unsigned int trimmedLength
        )
{
    //Snort may need to raise a separate event if
    //only trimmed length is tiny.
    if(p->mf)
    {
        ///detect tiny fragments before processing overlaps.
        if (f3context->min_fragment_length)
        {
            if (p->ip_frag_len <= f3context->min_fragment_length)
            {
                DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                            "Frag3: Received fragment size(%d) is not more than configured min_fragment_length (%d)\n",
                            p->ip_frag_len, f3context->min_fragment_length););
                EventTinyFragments(f3context);
                return 1;
            }

            ///detect tiny fragments after processing overlaps.
            if (trimmedLength <= f3context->min_fragment_length)
            {
                DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                            "Frag3: # of New octets in Received fragment(%d) is not more than configured min_fragment_length (%d)\n",
                            trimmedLength, f3context->min_fragment_length););
                EventTinyFragments(f3context);
                return 1;
            }
        }
    }

    return 0;
}

int  frag3DropAllFragments(
        Packet *p
        )
{
    FragTracker *ft = (FragTracker *)p->fragtracker;

    //drop this and all following fragments
    if (ft && !(ft->frag_flags & FRAG_DROP_FRAGMENTS))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                        "Frag3: Will drop all fragments on this packet\n"););
        ft->frag_flags |= FRAG_DROP_FRAGMENTS;
    }

    return 0;
}

/**
 * This is where the rubber hits the road.  Insert the new fragment's data
 * into the current FragTracker's fraglist, doing anomaly detection and
 * handling overlaps in a target-based manner.
 *
 * @param p Current packet to insert
 * @param ft FragTracker to hold the packet
 * @param fkey FragKey with the current FragTracker's key info
 * @param f3context context of the current engine for target-based defrag info
 *
 * @return status
 * @retval FRAG_INSERT_TIMEOUT FragTracker has timed out and been dropped
 * @retval FRAG_INSERT_ATTACK  Attack detected during insertion
 * @retval FRAG_INSERT_ANOMALY Anomaly detected during insertion
 * @retval FRAG_INSERT_TTL Delta of TTL values beyond configured value
 * @retval FRAG_INSERT_OK Fragment has been inserted successfully
 */
static int Frag3Insert(Packet *p, FragTracker *ft, FRAGKEY *fkey,
        Frag3Context *f3context)
{
    uint16_t orig_offset;    /* offset specified in this fragment header */
    uint16_t frag_offset;    /* calculated offset for this fragment */
    uint32_t frag_end;       /* calculated end point for this fragment */
    int16_t trunc = 0;      /* we truncate off the tail */
    int32_t overlap = 0;    /* we overlap on either end of the frag */
    int16_t len = 0;        /* calculated size of the fragment */
    int16_t slide = 0;      /* slide up the front of the current frag */
    int done = 0;           /* flag for right-side overlap handling loop */
    int addthis = 1;           /* flag for right-side overlap handling loop */
    int i = 0;              /* counter */
    int firstLastOk;
    int ret = FRAG_INSERT_OK;
    unsigned char lastfrag = 0; /* Set to 1 when this is the 'last' frag */
    unsigned char alerted_overlap = 0; /* Set to 1 when alerted */
    Frag3Frag *right = NULL; /* frag ptr for right-side overlap loop */
    Frag3Frag *newfrag = NULL;  /* new frag container */
    Frag3Frag *left = NULL;     /* left-side overlap fragment ptr */
    Frag3Frag *idx = NULL;      /* indexing fragment pointer for loops */
    Frag3Frag *dump_me = NULL;  /* frag ptr for complete overlaps to dump */
    const uint8_t *fragStart;
    int16_t fragLength;
    uint32_t reassembled_pkt_size;
    PROFILE_VARS;

    sfBase.iFragInserts++;

    PREPROC_PROFILE_START(frag3InsertPerfStats);

    if (IS_IP6(p) && (p->frag_offset == 0))
    {
        IP6Frag *fragHdr = (IP6Frag *)p->ip6_extensions[p->ip6_frag_index].data;
        if (ft->protocol != fragHdr->ip6f_nxt)
        {
            ft->protocol = fragHdr->ip6f_nxt;
        }
    }

    /*
     * Check to see if this fragment is the first or last one and
     * set the appropriate flags and values in the FragTracker
     */
    firstLastOk = Frag3CheckFirstLast(p, ft);

    fragStart = p->ip_frag_start;
    //fragStart = (uint8_t *)p->iph + GET_IPH_HLEN(p) * 4;
    /* Use the actual length here, because packet may have been
     * truncated.  Don't want to try to copy more than we actually
     * captured. */
    //len = fragLength = p->actual_ip_len - GET_IPH_HLEN(p) * 4;
    len = fragLength = p->ip_frag_len;
#ifdef DEBUG_MSGS
    if (p->actual_ip_len != ntohs(GET_IPH_LEN(p)))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
            "IP Actual Length (%d) != specified length (%d), "
            "truncated packet (%d)?\n",
            p->actual_ip_len, ntohs(GET_IPH_LEN(p)), pkt_snaplen););
    }
#endif

    /*
     * setup local variables for tracking this frag
     */
    orig_offset = frag_offset = p->frag_offset << 3;
    /* Reset the offset to handle the weird Solaris case */
    if (firstLastOk == FRAG_LAST_OFFSET_ADJUST)
        frag_offset = (uint16_t)ft->calculated_size;
    frag_end = frag_offset + fragLength;

    /*
     * Copy the calculated size of the reassembled
     * packet in a local variable.
     */
    reassembled_pkt_size = ft->calculated_size;

    /*
     * might have last frag...
     */
    if(!p->mf)
    {
        if ((frag_end > ft->calculated_size) &&
            (firstLastOk == FRAG_LAST_OFFSET_ADJUST))
        {
            ft->calculated_size = frag_end;
        }

        //    ft->frag_flags |= FRAG_GOT_LAST;
        //    ft->calculated_size = (p->frag_offset << 3) + fragLength;
        lastfrag = 1;
    }
    else
    {
        uint16_t oldfrag_end;
        /*
         * all non-last frags are supposed to end on 8-byte boundries
         */
        if(frag_end & 7)
        {
            /*
             * bonk/boink/jolt/etc attack...
             */
            DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                        "[..] Short frag (Bonk, etc) attack!\n"););

            EventAnomShortFrag(f3context);

            /* don't return, might still be interesting... */
        }

        /* can't have non-full fragments... */
        oldfrag_end = frag_end;
        frag_end &= ~7;

        /* Adjust len to take into account the jolting/non-full fragment. */
        len -= (oldfrag_end - frag_end);

        /*
         * if the end of this frag is greater than the max frag size we have a
         * problem
         */
        if(frag_end > ft->calculated_size)
        {
            if(ft->frag_flags & FRAG_GOT_LAST)
            {
                /* oversize frag attack */
                DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                            "[..] Oversize frag pkt!\n"););

                EventAnomOversize(f3context);

                PREPROC_PROFILE_END(frag3InsertPerfStats);
                return FRAG_INSERT_ANOMALY;
            }
            ft->calculated_size = frag_end;
        }
    }

    if(frag_end == frag_offset)
    {
        /*
         * zero size frag...
         */
        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                    "[..] Zero size frag!\n"););

        if(f3context->frag3_alerts & FRAG3_DETECT_ANOMALIES)
        {
            EventAnomZeroFrag(f3context);
        }

        PREPROC_PROFILE_END(frag3InsertPerfStats);
        return FRAG_INSERT_ANOMALY;
    }

    if(frag_end > IP_MAXPACKET)
    {
        /*
         * oversize pkt...
         */
        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                    "[..] Oversize frag!\n"););

            EventAnomBadsizeLg(f3context);

        ft->frag_flags |= FRAG_BAD;

	/*
         * Restore the value of ft->calculated_size
         */
        ft->calculated_size = reassembled_pkt_size;

        PREPROC_PROFILE_END(frag3InsertPerfStats);
        return FRAG_INSERT_ANOMALY;
    }

    /*
     * This may alert on bad options, but we still want to
     * insert the packet
     */
    Frag3HandleIPOptions(ft, p);

    ft->frag_pkts++;

    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                "Walking frag list (%d nodes), new frag %d@%d\n",
                ft->fraglist_count, fragLength, frag_offset););

    /*
     * Need to figure out where in the frag list this frag should go
     * and who its neighbors are
     */
    for(idx = ft->fraglist; idx; idx = idx->next)
    {
        i++;
        right = idx;

        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                    "%d right o %d s %d ptr %p prv %p nxt %p\n",
                    i, right->offset, right->size, right,
                    right->prev, right->next););

        if(right->offset >= frag_offset)
        {
            break;
        }

        left = right;
    }

    /*
     * null things out if we walk to the end of the list
     */
    if(idx == NULL) right = NULL;

    /*
     * handle forward (left-side) overlaps...
     */
    if(left)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                    "Dealing with previous (left) frag %d@%d\n",
                    left->size, left->offset););

        /*
         * generate the overlap of the current packet fragment
         * over this left-side fragment
         */
        /* NOTE: If frag_offset is really large, overlap can be
         * negative because its stored as a 32bit int.
         */
        overlap = left->offset + left->size - frag_offset;

        if(overlap > 0)
        {
            f3stats.overlaps++;
            ft->overlap_count++;

            if(frag_end < ft->calculated_size ||
                    ((ft->frag_flags & FRAG_GOT_LAST) &&
                     frag_end != ft->calculated_size))
            {
                if (!p->mf)
                {
                    /*
                     * teardrop attack...
                     */
                    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                                "[..] Teardrop attack!\n"););

                    EventAttackTeardrop(f3context);

                    ft->frag_flags |= FRAG_BAD;

                    PREPROC_PROFILE_END(frag3InsertPerfStats);
                    return FRAG_INSERT_ATTACK;
                }
            }

            /*
             * Ok, we've got an overlap so we need to handle it.
             *
             * The target-based modes here match the data generated by
             * Paxson's Active Mapping paper as do the policy types.
             */
            switch(ft->frag_policy)
            {
                /*
                 * new frag gets moved around
                 */
                case FRAG_POLICY_LINUX:
                case FRAG_POLICY_FIRST:
                case FRAG_POLICY_WINDOWS:
                case FRAG_POLICY_SOLARIS:
                case FRAG_POLICY_BSD:
                    frag_offset += (int16_t)overlap;
                    slide = (int16_t)overlap;

                    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                                "left overlap, new frag moves: %d bytes, "
                                "slide: %d\n", overlap, slide););

                    if(frag_end <= frag_offset)
                    {
                        /*
                         * zero size frag
                         */
                        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                                    "zero size frag\n"););

                        EventAnomZeroFrag(f3context);

                        PREPROC_PROFILE_END(frag3InsertPerfStats);
                        return FRAG_INSERT_ANOMALY;
                    }

                    DEBUG_WRAP(DebugMessage(DEBUG_FRAG, "left overlap, "
                                "truncating new pkt (slide: %d)\n", slide););

                    break;

                    /*
                     * new frag stays where it is, overlapee (existing frag)
                     * gets whacked
                     */
                case FRAG_POLICY_BSD_RIGHT:
                    if (left->offset + left->size >= frag_offset + len)
                    {
                        /* BSD-right (HP Printers) favor new fragments with
                         * lower/equal offset, EXCEPT when the existing
                         * fragment ends with at a higher/equal offset.
                         */
                        frag_offset += (int16_t)overlap;
                        slide = (int16_t)overlap;
                        goto left_overlap_last;
                    }
                    /* fall through */
                case FRAG_POLICY_LAST:
                    if ((left->offset < frag_offset) && (left->offset + left->size > frag_offset + len))
                    {
                        /* The new frag is overlapped on both sides by an
                         * existing frag -- existing frag needs to be split
                         * and the new frag inserted in the middle.
                         *
                         * Need to duplciate left.  Adjust that guys
                         * offset by + (frag_offset + len) and
                         * size by - (frag_offset + len - left->offset).
                         */
                        ret = DupFragNode(ft, left, &right);
                        if (ret != FRAG_INSERT_OK)
                        {
                            /* Some warning here,
                             * no, its done in AddFragNode */
                            PREPROC_PROFILE_END(frag3InsertPerfStats);
                            return ret;
                        }
                        left->size -= (int16_t)overlap;
                        ft->frag_bytes -= (int16_t)overlap;

                        right->offset = frag_offset + len;
                        right->size -= (frag_offset + len - left->offset);
                        right->data += (frag_offset + len - left->offset);
                        ft->frag_bytes -= (frag_offset + len - left->offset);
                    }
                    else
                    {
                        left->size -= (int16_t)overlap;
                        ft->frag_bytes -= (int16_t)overlap;
                    }

left_overlap_last:
                    DEBUG_WRAP(DebugMessage(DEBUG_FRAG, "[!!] left overlap, "
                                "truncating old pkt (offset: %d overlap: %d)\n",
                                left->offset, overlap););

                    if (left->size <= 0)
                    {
                        dump_me = left;

                        DEBUG_WRAP(DebugMessage(DEBUG_FRAG, "retrans, "
                                "dumping old frag (offset: %d overlap: %d)\n",
                                dump_me->offset, overlap););

                        left = left->prev;

                        Frag3FraglistDeleteNode(ft, dump_me);
                    }

                    break;
            }

            /*
             * frag can't end before it begins...
             */
            if(frag_end < frag_offset)
            {
                DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                            "frag_end < frag_offset!"););

                if(f3context->frag3_alerts & FRAG3_DETECT_ANOMALIES)
                {
                    EventAnomBadsizeSm(f3context);
                }

                PREPROC_PROFILE_END(frag3InsertPerfStats);
                return FRAG_INSERT_ANOMALY;
            }
        }
        else
        {
            DEBUG_WRAP(DebugMessage(DEBUG_FRAG, "No left overlap!\n"););
        }
    }

    if ((uint16_t)fragLength > pkt_snaplen)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                    "Overly large fragment %d 0x%x 0x%x %d\n",
                    fragLength, GET_IPH_LEN(p), GET_IPH_OFF(p),
                    p->frag_offset << 3););
        PREPROC_PROFILE_END(frag3InsertPerfStats);
        return FRAG_INSERT_FAILED;
    }

    /*
     * handle tail (right-side) overlaps
     *
     * We have to walk thru all the right side frags until the offset of the
     * existing frag is greater than the end of the new frag
     */
    while(right && (right->offset < frag_end) && !done)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                    "Next (right)fragment %d@%d\n",
                    right->size, right->offset););

#ifdef DEBUG_FRAG3
        PrintFrag3Frag(right);
#endif
        trunc = 0;
        overlap = frag_end - right->offset;

        if (overlap)
        {
            if(frag_end < ft->calculated_size ||
                    ((ft->frag_flags & FRAG_GOT_LAST) &&
                     frag_end != ft->calculated_size))
            {
                if (!p->mf)
                {
                    /*
                     * teardrop attack...
                     */
                    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                                "[..] Teardrop attack!\n"););

                    EventAttackTeardrop(f3context);

                    ft->frag_flags |= FRAG_BAD;

                    PREPROC_PROFILE_END(frag3InsertPerfStats);
                    return FRAG_INSERT_ATTACK;
                }
            }
        }

        /*
         * partial right-side overlap, this will be the last frag to check
         */
        if(overlap < right->size)
        {
            f3stats.overlaps++;
            ft->overlap_count++;

            DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                        "Right-side overlap %d bytes\n", overlap););

            /*
             * once again, target-based policy processing
             */
            switch(ft->frag_policy)
            {
                /*
                 * existing fragment gets truncated
                 */
                case FRAG_POLICY_LAST:
                case FRAG_POLICY_LINUX:
                case FRAG_POLICY_BSD:
                    if ((ft->frag_policy == FRAG_POLICY_BSD) &&
                        (right->offset == frag_offset))
                    {
                        slide = (int16_t)(right->offset + right->size - frag_offset);
                        frag_offset += (int16_t)slide;
                    }
                    else
                    {
                        right->offset += (int16_t)overlap;
                        right->data += (int16_t)overlap;
                        right->size -= (int16_t)overlap;
                        ft->frag_bytes -= (int16_t)overlap;
                    }
                    DEBUG_WRAP(DebugMessage(DEBUG_FRAG, "[!!] right overlap, "
                                "truncating old frag (offset: %d, "
                                "overlap: %d)\n", right->offset, overlap);
                            DebugMessage(DEBUG_FRAG,
                                "Exiting right overlap loop...\n"););
                    if (right->size <= 0)
                    {
                        dump_me = right;

                        DEBUG_WRAP(DebugMessage(DEBUG_FRAG, "retrans, "
                                "dumping old frag (offset: %d overlap: %d)\n",
                                dump_me->offset, overlap););

                        right = right->next;

                        Frag3FraglistDeleteNode(ft, dump_me);
                    }
                    break;

                /*
                 * new frag gets truncated
                 */
                case FRAG_POLICY_FIRST:
                case FRAG_POLICY_WINDOWS:
                case FRAG_POLICY_SOLARIS:
                case FRAG_POLICY_BSD_RIGHT:
                    trunc = (int16_t)overlap;
                    DEBUG_WRAP(DebugMessage(DEBUG_FRAG, "[!!] right overlap, "
                                "truncating new frag (offset: %d "
                                "overlap: %d)\n",
                                right->offset, overlap);
                            DebugMessage(DEBUG_FRAG,
                                "Exiting right overlap loop...\n"););
                    break;
            }

            /*
             * all done, bail
             */
            done = 1;
        }
        else
        {
            /*
             * we've got a full overlap
             */
            if(!alerted_overlap && (f3context->frag3_alerts & FRAG3_DETECT_ANOMALIES))
            {
                /*
                 * retrans/full overlap
                 */
                EventAnomOverlap(f3context);
                alerted_overlap = 1;
                f3stats.overlaps++;
                ft->overlap_count++;
            }

            /*
             * handle the overlap in a target-based manner
             */
            switch(ft->frag_policy)
            {
                /*
                 * overlap is treated differently if there is more
                 * data beyond the overlapped packet.
                 */
                case FRAG_POLICY_WINDOWS:
                case FRAG_POLICY_SOLARIS:
                case FRAG_POLICY_BSD:
                    /*
                     * Old packet is overlapped on both sides...
                     * Drop the old packet.  This follows a
                     * POLICY_LAST model.
                     */
                    if ((frag_end > right->offset + right->size) &&
                        (frag_offset < right->offset))
                    {
                        dump_me = right;
                        ft->frag_bytes -= right->size;

                        DEBUG_WRAP(DebugMessage(DEBUG_FRAG, "retrans, "
                                "dumping old frag (offset: %d overlap: %d)\n",
                                dump_me->offset, overlap););

                        right = right->next;

                        Frag3FraglistDeleteNode(ft, dump_me);
                        break;
                    }
                    else
                    {
                        if ((ft->frag_policy == FRAG_POLICY_SOLARIS) ||
                            (ft->frag_policy == FRAG_POLICY_BSD))
                        {
                            /* SOLARIS & BSD only */
                            if ((frag_end == right->offset + right->size) &&
                                (frag_offset < right->offset))
                            {
                                /* If the frag overlaps an entire frag to the
                                 * right side of that frag, the old frag if
                                 * dumped -- this is a "policy last".
                                 */
                                goto right_overlap_last;
                            }
                        }
                    }
                    /* Otherwise, treat it as a POLICY_FIRST,
                     * and trim accordingly. */

                    /* ie, fall through to the next case */

                /*
                 * overlap is rejected
                 */
                case FRAG_POLICY_FIRST:
                    /* fix for bug 17823 */
                    if (right->offset == frag_offset)
                    {
                        slide = (int16_t)(right->offset + right->size - frag_offset);
                        frag_offset += (int16_t)slide;
                        left = right;
                        right = right->next;
                    }
                    else
                    {
                        trunc = (int16_t)overlap;
                    }

                    DEBUG_WRAP(DebugMessage(DEBUG_FRAG, "right overlap, "
                                "rejecting new overlap data (overlap: %d, "
                                "trunc: %d)\n", overlap, trunc););

                    if (frag_end - trunc <= frag_offset)
                    {
                        /*
                         * zero size frag
                         */
                        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                                    "zero size frag (len: %d  overlap: %d)\n",
                                    fragLength, overlap););

                        f3stats.discards++;

                        PREPROC_PROFILE_END(frag3InsertPerfStats);
                        return FRAG_INSERT_ANOMALY;
                    }

                    {
                        uint16_t curr_end;
                        /* Full overlapping an already received packet
                         * and there are more packets beyond that fully
                         * overlapped one.
                         * Arrgh.  Need to insert this guy in chunks.
                         */
                        checkTinyFragments(f3context, p, len-slide-trunc);

                        ret = AddFragNode(ft, p, f3context, fragStart, fragLength, 0, len,
                                slide, trunc, frag_offset, left, &newfrag);
                        if (ret != FRAG_INSERT_OK)
                        {
                            /* Some warning here,
                             * no, its done in AddFragNode */
                            PREPROC_PROFILE_END(frag3InsertPerfStats);
                            return ret;
                        }

                        curr_end = newfrag->offset + newfrag->size;

                        /* Find the next gap that this one might fill in */
                        while (right &&
                            (curr_end == right->offset) &&
                            (right->offset < frag_end))
                        {
                            curr_end = right->offset + right->size;
                            left = right;
                            right = right->next;
                        }

                        if (right && (right->offset < frag_end))
                        {
                            /* Adjust offset to end of 'left' */
                            if (left)
                                frag_offset = left->offset + left->size;
                            else
                                frag_offset = orig_offset;

                            /* Overlapping to the left by a good deal now */
                            slide = frag_offset - orig_offset;
                            /*
                             * Reset trunc, in case the next one kicks us
                             * out of the loop.  This packet will become the
                             * right-most entry so far.  Don't truncate any
                             * further.
                             */
                            trunc = 0;
                            if (right)
                                continue;
                        }

                        if (curr_end < frag_end)
                        {
                            /* Insert this guy in his proper spot,
                             * adjust offset to the right-most endpoint
                             * we saw.
                             */
                            slide = left->offset + left->size - frag_offset;
                            frag_offset = curr_end;
                            trunc = 0;
                        }
                        else
                        {
                            addthis = 0;
                        }
                    }
                    break;

                    /*
                     * retrans accepted, dump old frag
                     */
right_overlap_last:
                case FRAG_POLICY_BSD_RIGHT:
                case FRAG_POLICY_LAST:
                case FRAG_POLICY_LINUX:
                    dump_me = right;
                    ft->frag_bytes -= right->size;

                    DEBUG_WRAP(DebugMessage(DEBUG_FRAG, "retrans, "
                                "dumping old frag (offset: %d overlap: %d)\n",
                                dump_me->offset, overlap););

                    right = right->next;

                    Frag3FraglistDeleteNode(ft, dump_me);

                    break;
            }
        }
    }

    ///detect tiny fragments but continue processing
    checkTinyFragments(f3context, p, len-slide-trunc);

    if ((f3context->overlap_limit) &&
            (ft->overlap_count >= f3context->overlap_limit))
    {
        //overlap limit exceeded. Raise event on all subsequent fragments
        DEBUG_WRAP(DebugMessage(DEBUG_FRAG, "Reached overlap limit.\n"););

        EventExcessiveOverlap(f3context);

        PREPROC_PROFILE_END(frag3InsertPerfStats);
        return FRAG_INSERT_OVERLAP_LIMIT;
    }

    if (addthis)
    {
        ret = AddFragNode(ft, p, f3context, fragStart, fragLength, lastfrag, len,
                      slide, trunc, frag_offset, left, &newfrag);
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                    "Fully truncated right overlap\n"););
    }

    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                "Frag3Insert(): returning normally\n"););

    PREPROC_PROFILE_END(frag3InsertPerfStats);
    return ret;
}

/**
 * Check to see if a FragTracker has met all of its completion criteria
 *
 * @param ft FragTracker to check
 *
 * @return status
 * @retval 1 If the FragTracker is ready to be rebuilt
 * @retval 0 If the FragTracker hasn't fulfilled its completion criteria
 */
static inline int Frag3IsComplete(FragTracker *ft)
{
    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                "[$] Checking completion criteria\n"););

    /*
     * check to see if the first and last frags have arrived
     */
    if((ft->frag_flags & FRAG_GOT_FIRST) &&
            (ft->frag_flags & FRAG_GOT_LAST))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                    "   Got First and Last frags\n"););

        /*
         * if we've accumulated enough data to match the calculated size
         * of the defragg'd packet, return 1
         */
        if(ft->frag_bytes == ft->calculated_size)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                        "   [!] frag_bytes = calculated_size!\n"););

            sfBase.iFragCompletes++;

            return 1;
        }

        if (ft->frag_bytes > ft->calculated_size)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                        "   [!] frag_bytes > calculated_size!\n"););

            sfBase.iFragCompletes++;

            return 1;
        }

        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                    "   Calc size (%d) != frag bytes (%d)\n",
                    ft->calculated_size, ft->frag_bytes););

        /*
         * no dice
         */
        return 0;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                "   Missing First or Last frags (frag_flags: 0x%X)\n",
                ft->frag_flags););

    return 0;
}

/**
 * Reassemble the packet from the data in the FragTracker and reinject into
 * Snort's packet analysis system
 *
 * @param ft FragTracker to rebuild
 * @param p Packet to fill in pseudopacket IP structs
 *
 * @return none
 */
static void Frag3Rebuild(FragTracker *ft, Packet *p)
{
    uint8_t *rebuild_ptr = NULL;  /* ptr to the start of the reassembly buffer */
    const uint8_t *rebuild_end;  /* ptr to the end of the reassembly buffer */
    Frag3Frag *frag;    /* frag pointer for managing fragments */
    int ret = 0;
    Packet* dpkt;
    PROFILE_VARS;

// XXX NOT YET IMPLEMENTED - debugging

    PREPROC_PROFILE_START(frag3RebuildPerfStats);

#ifdef GRE
    if ( p->encapsulated )
        dpkt = encap_defrag_pkt;
    else
#endif
        dpkt = defrag_pkt;

    Encode_Format(ENC_FLAG_DEF|ENC_FLAG_FWD, p, dpkt, PSEUDO_PKT_IP);
    /*
     * set the pointer to the end of the rebuild packet
     */
    rebuild_ptr = (uint8_t*)dpkt->data;
    // the encoder ensures enough space for a maximum datagram
    rebuild_end = (uint8_t*)dpkt->data + IP_MAXPACKET;

    if (IS_IP4(p))
    {
        /*
         * if there are IP options, copy those in as well
         * these are for the inner IP...
         */
        if (ft->ip_options_data && ft->ip_options_len)
        {
            /* Adjust the IP header size in pseudo packet for the new length */
            uint8_t new_ip_hlen = sizeof(*dpkt->iph) + ft->ip_options_len;

            DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                    "Adjusting IP Header to %d bytes\n",
                    new_ip_hlen););
            SET_IP_HLEN((IPHdr *)dpkt->iph, new_ip_hlen>>2);

            ret = SafeMemcpy(rebuild_ptr, ft->ip_options_data,
                ft->ip_options_len, rebuild_ptr, rebuild_end);

            if (ret == SAFEMEM_ERROR)
            {
                /*XXX: Log message, failed to copy */
                ft->frag_flags = ft->frag_flags | FRAG_REBUILT;
                return;
            }
            rebuild_ptr += ft->ip_options_len;
        }
        else if (ft->copied_ip_options_len)
        {
            /* XXX: should we log a warning here?  there were IP options
             * copied across all fragments, EXCEPT the offset 0 fragment.
             */
        }

        /*
         * clear the packet fragment fields
         */
        ((IPHdr *)dpkt->iph)->ip_off = 0x0000;
        dpkt->frag_flag = 0;

        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                    "[^^] Walking fraglist:\n"););
    }

    /*
     * walk the fragment list and rebuild the packet
     */
    for(frag = ft->fraglist; frag; frag = frag->next)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                    "   frag: %p\n"
                    "   frag->data: %p\n"
                    "   frag->offset: %d\n"
                    "   frag->size: %d\n"
                    "   frag->prev: %p\n"
                    "   frag->next: %p\n",
                    frag, frag->data, frag->offset,
                    frag->size, frag->prev, frag->next););

        /*
         * We somehow got a frag that had data beyond the calculated
         * end. Don't want to include it.
         */
        if ((frag->offset + frag->size) > (uint16_t)ft->calculated_size)
            continue;

        /*
         * try to avoid buffer overflows...
         */
        if (frag->size)
        {
            ret = SafeMemcpy(rebuild_ptr+frag->offset, frag->data, frag->size,
                             rebuild_ptr, rebuild_end);

            if (ret == SAFEMEM_ERROR)
            {
                /*XXX: Log message, failed to copy */
                ft->frag_flags = ft->frag_flags | FRAG_REBUILT;
                return;
            }
        }
    }

    if (IS_IP4(p))
    {
        /*
         * tell the rest of the system that this is a rebuilt fragment
         */
        dpkt->packet_flags |= PKT_REBUILT_FRAG;
        dpkt->frag_flag = 0;
        dpkt->dsize = (uint16_t)ft->calculated_size;

        Encode_Update(dpkt);
    }
    else /* Inner/only is IP6 */
    {
        IP6RawHdr* rawHdr = (IP6RawHdr*)dpkt->raw_ip6h;

        if ( !rawHdr )
        {
            /*XXX: Log message, failed to copy */
            ft->frag_flags = ft->frag_flags | FRAG_REBUILT;
            return;
        }

        /* IPv6 Header is already copied over, as are all of the extensions
         * that were not part of the fragmented piece. */

        /* Set the 'next' protocol */
        if (p->ip6_frag_index > 0)
        {
            // FIXTHIS use of last_extension works but is ugly
            IP6Extension *last_extension = (IP6Extension *)
                (dpkt->pkt + (p->ip6_extensions[p->ip6_frag_index -1].data - p->pkt));
            last_extension->ip6e_nxt = ft->protocol;
        }
        else
        {
            rawHdr->ip6nxt = ft->protocol;
        }
        dpkt->dsize = (uint16_t)ft->calculated_size;
        Encode_Update(dpkt);
    }

    pc.rebuilt_frags++;
    sfBase.iFragFlushes++;

    /* Rebuild is complete */
    PREPROC_PROFILE_END(frag3RebuildPerfStats);

    /*
     * process the packet through the detection engine
     */
    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                "Processing rebuilt packet:\n"););

    f3stats.reassembles++;

    UpdateIPReassStats(&sfBase, dpkt->pkth->caplen);

#if defined(DEBUG_FRAG3) && defined(DEBUG)
    /*
     * Note, that this won't print out the IP Options or any other
     * data that is established when the packet is decoded.
     */
    if (DEBUG_FRAG & GetDebugLevel())
    {
        //ClearDumpBuf();
        printf("++++++++++++++++++Frag3 DEFRAG'd PACKET++++++++++++++\n");
        PrintIPPkt(stdout, dpkt->iph->ip_proto, &dpkt);
        printf("++++++++++++++++++Frag3 DEFRAG'd PACKET++++++++++++++\n");
        //ClearDumpBuf();
    }
#endif
    SnortEventqPush();
    ProcessPacket(dpkt, dpkt->pkth, dpkt->pkt, ft);
    SnortEventqPop();

    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                "Done with rebuilt packet, marking rebuilt...\n"););

    ft->frag_flags = ft->frag_flags | FRAG_REBUILT;
}

/**
 * Delete a Frag3Frag struct
 *
 * @param frag Fragment to delete
 *
 * @return none
 */
static void Frag3DeleteFrag(Frag3Frag *frag)
{
    /*
     * delete the fragment either in prealloc or dynamic mode
     */
    if(!frag3_eval_config->use_prealloc)
    {
        SnortPreprocFree(frag->fptr, frag->flen, PP_FRAG3, PP_MEM_CATEGORY_SESSION);
        frag3_mem_in_use -= frag->flen;

        SnortPreprocFree(frag, sizeof(Frag3Frag), PP_FRAG3, PP_MEM_CATEGORY_SESSION);
        frag3_mem_in_use -= sizeof(Frag3Frag);

        sfBase.frag3_mem_in_use = frag3_mem_in_use;
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FRAG, "o %d s %d ptr %p prv %p nxt %p\n",
                    frag->offset, frag->size, frag, frag->prev, frag->next););
        Frag3PreallocPush(frag);
    }

    f3stats.fragnodes_released++;
}

/**
 * Delete the contents of a FragTracker, in this instance that just means to
 * dump the fraglist.  The sfxhash system deletes the actual FragTracker mem.
 *
 * @param ft FragTracker to delete
 *
 * @return none
 */
static void Frag3DeleteTracker(FragTracker *ft)
{
    Frag3Frag *idx = ft->fraglist;  /* pointer to the fraglist to delete */
    Frag3Frag *dump_me = NULL;      /* ptr to the Frag3Frag element to drop */

    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                "Frag3DeleteTracker %d nodes to dump\n", ft->fraglist_count););

    /*
     * delete all the nodes in a fraglist
     */
    while(idx)
    {
        dump_me = idx;
        idx = idx->next;
        Frag3DeleteFrag(dump_me);
    }
    ft->fraglist = NULL;
    if (ft->ip_options_data)
    {
        SnortPreprocFree(ft->ip_options_data, ft->ip_options_len, PP_FRAG3,
                 PP_MEM_CATEGORY_SESSION);
        ft->ip_options_data = NULL;
    }

    return;
}

/**
 * Remove a FragTracker from the f_cache hash table
 *
 * @param key FragKey of the FragTracker to be removed
 * @param data unused in this function
 *
 * @return none
 */
static void Frag3RemoveTracker(void *key, void *data)
{
    /*
     * sfxhash maintains its own self preservation stuff/node freeing stuff
     */
    if(sfxhash_remove(f_cache, key) != SFXHASH_OK)
    {
        ErrorMessage("sfxhash_remove() failed in frag3!\n");
    }

    return;
}

/**
 * This is the auto-node-release function that gets handed to the sfxhash table
 * at initialization.  Handles deletion of sfxhash table data members.
 *
 * @param key FragKey of the element to be freed
 * @param data unused in this implementation
 *
 * Now Returns 0 because we want to say, yes, delete that hash entry!!!
 */
static int Frag3AutoFree(void *key, void *data)
{
    FragTracker *ft = (FragTracker *)data;
    tSfPolicyUserContextId config;
    tSfPolicyId policy_id;
    Frag3Config *pPolicyConfig = NULL;

    if (ft == NULL)
        return 0;

    config = ft->config;
    policy_id = ft->policy_id;
    pPolicyConfig = (Frag3Config *)sfPolicyUserDataGet(config, policy_id);

    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                "Calling Frag3DeleteTracker()\n"););

    Frag3DeleteTracker(ft);

    sfBase.iFragDeletes++;
    sfBase.iFragAutoFrees++;
    sfBase.iCurrentFrags--;
    f3stats.fragtrackers_autoreleased++;

    if (pPolicyConfig != NULL)
    {
        pPolicyConfig->ref_count--;
        if ((pPolicyConfig->ref_count == 0) &&
            (config != frag3_config))
        {
            Frag3FreeConfig(pPolicyConfig);
            sfPolicyUserDataClear (config, policy_id);

            /* No more outstanding policies for this config */
            if (sfPolicyUserPolicyGetActive(config) == 0)
                Frag3FreeConfigs(config);
        }
    }

    return 0;
}

/**
 * This is the user free function that gets handed to the sfxhash table
 * at initialization.  Handles deletion of sfxhash table data members.
 *
 * @param key FragKey of the element to be freed
 * @param data unused in this implementation
 *
 * Now Returns 0 because we want to say, yes, delete that hash entry!!!
 */
static int Frag3UserFree(void *key, void *data)
{
    FragTracker *ft = (FragTracker *)data;
    tSfPolicyUserContextId config;
    tSfPolicyId policy_id;
    Frag3Config *pPolicyConfig = NULL;

    if (ft == NULL)
        return 0;

    config = ft->config;
    policy_id = ft->policy_id;
    pPolicyConfig = (Frag3Config *)sfPolicyUserDataGet(config, policy_id);

    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                "Calling Frag3DeleteTracker()\n"););

    Frag3DeleteTracker(ft);

    sfBase.iFragDeletes++;
    sfBase.iCurrentFrags--;
    f3stats.fragtrackers_released++;

    if (pPolicyConfig != NULL)
    {
        pPolicyConfig->ref_count--;
        if ((pPolicyConfig->ref_count == 0) &&
            (config != frag3_config))
        {
            Frag3FreeConfig(pPolicyConfig);
            sfPolicyUserDataClear (config, policy_id);

            /* No more outstanding policies for this config */
            if (sfPolicyUserPolicyGetActive(config) == 0)
                Frag3FreeConfigs(config);
        }
    }

    return 0;
}

/**
 * This function gets called either when we run out of prealloc nodes or when
 * the memcap is exceeded.  Its job is to free memory up in frag3 by deleting
 * old/stale data.  Currently implemented using a simple LRU pruning
 * technique, could probably benefit from having some sort of tail selection
 * randomization added to it.  Additonally, right now when we hit the wall we
 * try to drop at least enough memory to satisfy the "ten_percent" value.
 * Hopefully that's not too aggressive, salt to taste!
 *
 * @param none
 *
 * @return none
 */
static int Frag3Prune(FragTracker *not_me)
{
    SFXHASH_NODE *hnode;
    int found_this = 0;
    int pruned = 0;
#ifdef DEBUG
    /* Use these to print out whether the frag tracker has
     * expired or not.
     */
    FragTracker *ft;
    struct timeval *fttime;     /* FragTracker timestamp */
#endif

    sfBase.iFragFaults++;
    f3stats.prunes++;

    if(!frag3_eval_config->use_prealloc)
    {
        //while(frag3_mem_in_use > (frag3_eval_config->memcap-globa_config->ten_percent))
        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                    "(spp_frag3) Frag3Prune: Pruning by memcap! "););
        while((frag3_mem_in_use > frag3_eval_config->memcap) ||
              (f_cache->count > (frag3_eval_config->max_frags - 5)))
        {
            hnode = sfxhash_lru_node(f_cache);
            if(!hnode)
            {
                break;
            }

            if (hnode && hnode->data == not_me)
            {
                if (found_this)
                {
                    /* Uh, problem... we've gone through the entire list */
                    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                        "(spp_frag3) Frag3Prune: Pruning by memcap - empty list! "););
                    return pruned;
                }
                sfxhash_gmovetofront(f_cache, hnode);
                found_this = 1;
                continue;
            }
#ifdef DEBUG
            ft = hnode->data;
            fttime = &(ft->frag_time);

            if (CheckTimeout(pkttime,fttime,ft->context)==FRAG_TIMEOUT)
            {
                char *src_str = SnortStrdup(FragIPToStr(ft->sip, ft->ipver));
                LogMessage("(spp_frag3) Frag3Prune: Fragment dropped (timeout)! "
                    "[%s->%s ID: %d Count: %d]\n", src_str, FragIPToStr(ft->dip, ft->ipver),
                    ft->id, ft->fraglist_count);
                free(src_str);
                f3stats.timeouts++;
                sfBase.iFragTimeouts++;
            }
            else
            {
                char *src_str = SnortStrdup(FragIPToStr(ft->sip, ft->ipver));
                LogMessage("(spp_frag3) Frag3Prune: Fragment dropped (memory)! "
                    "[%s->%s ID: %d Count: %d]\n", src_str, FragIPToStr(ft->dip, ft->ipver),
                    ft->id, ft->fraglist_count);
                free(src_str);
            }
#endif
            Frag3RemoveTracker(hnode->key, hnode->data);
            //sfBase.iFragDeletes++;
            //f3stats.fragtrackers_released++;
            pruned++;
        }
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                    "(spp_frag3) Frag3Prune: Pruning by prealloc! "););
        while (prealloc_nodes_in_use >
               (frag3_eval_config->static_frags - frag3_eval_config->ten_percent))
        {
            hnode = sfxhash_lru_node(f_cache);
            if(!hnode)
            {
                break;
            }

            if (hnode && hnode->data == not_me)
            {
                if (found_this)
                {
                    /* Uh, problem... we've gone through the entire list */
                    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                              "(spp_frag3) Frag3Prune: Pruning by prealloc - empty list! "););
                    return pruned;
                }
                sfxhash_gmovetofront(f_cache, hnode);
                found_this = 1;
                continue;
            }

#ifdef DEBUG
            ft = hnode->data;
            fttime = &(ft->frag_time);

            if (CheckTimeout(pkttime,fttime,ft->context)==FRAG_TIMEOUT)
            {
                char *src_str = SnortStrdup(FragIPToStr(ft->sip, ft->ipver));
                LogMessage("(spp_frag3) Frag3Prune: Fragment dropped (timeout)! "
                    "[%s->%s ID: %d Count: %d]\n", src_str, FragIPToStr(ft->dip, ft->ipver),
                    ft->id, ft->fraglist_count);
                free(src_str);
                f3stats.timeouts++;
                sfBase.iFragTimeouts++;
            }
            else
            {
                char *src_str = SnortStrdup(FragIPToStr(ft->sip, ft->ipver));
                LogMessage("(spp_frag3) Frag3Prune: Fragment dropped (memory)! "
                    "[%s->%s ID: %d Count: %d]\n", src_str, FragIPToStr(ft->dip, ft->ipver),
                    ft->id, ft->fraglist_count);
                free(src_str);
            }
#endif

            Frag3RemoveTracker(hnode->key, hnode->data);
            //sfBase.iFragDeletes++;
            //f3stats.fragtrackers_released++;
            pruned++;
        }
    }

    DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                "(spp_frag3) Frag3Prune: Pruned %d nodes\n", pruned););
    return pruned;
}

/**
 * Print out the frag stats from this run
 *
 * @param none
 *
 * @return none
 */
static void Frag3PrintStats(int exiting)
{
    LogMessage("Frag3 statistics:\n");
    LogMessage("        Total Fragments: %u\n", f3stats.total);
    LogMessage("      Frags Reassembled: %u\n", f3stats.reassembles);
    LogMessage("               Discards: %u\n", f3stats.discards);
    LogMessage("          Memory Faults: %u\n", f3stats.prunes);
    LogMessage("               Timeouts: %u\n", f3stats.timeouts);
    LogMessage("               Overlaps: %u\n", f3stats.overlaps);
    LogMessage("              Anomalies: %u\n", f3stats.anomalies);
    LogMessage("                 Alerts: %u\n", f3stats.alerts);
    LogMessage("                  Drops: %u\n", f3stats.drops);
    LogMessage("     FragTrackers Added: %u\n", f3stats.fragtrackers_created);
    LogMessage("    FragTrackers Dumped: %u\n", f3stats.fragtrackers_released);
    LogMessage("FragTrackers Auto Freed: %u\n", f3stats.fragtrackers_autoreleased);
    LogMessage("    Frag Nodes Inserted: %u\n", f3stats.fragnodes_created);
    LogMessage("     Frag Nodes Deleted: %u\n", f3stats.fragnodes_released);
}

static int Frag3FreeConfigsPolicy(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    Frag3Config *pPolicyConfig = (Frag3Config *)pData;

    //do any housekeeping before freeing Frag3Config
    sfPolicyUserDataClear (config, policyId);
    Frag3FreeConfig(pPolicyConfig);

    return 0;
}

static void Frag3FreeConfigs(tSfPolicyUserContextId config)
{
    if (config == NULL)
        return;

    sfPolicyUserDataFreeIterate (config, Frag3FreeConfigsPolicy);

    sfPolicyConfigDelete(config);
}

static void Frag3FreeConfig(Frag3Config *config)
{
    int engineIndex;
    Frag3Context *f3context;

    if (config == NULL)
        return;

    /* Cleanup the list of Frag3 engine contexts */
    for (engineIndex = 0; engineIndex < config->numFrag3Contexts; engineIndex++)
    {
        f3context = config->frag3ContextList[engineIndex];
        if (f3context->bound_addrs != NULL)
        {
            sfvar_free(f3context->bound_addrs);
        }

        SnortPreprocFree(f3context, sizeof(Frag3Context), PP_FRAG3,
                  PP_MEM_CATEGORY_CONFIG);
    }

    if (config->frag3ContextList != NULL)
        SnortPreprocFree(config->frag3ContextList,
                 config->numFrag3Contexts * sizeof (Frag3Context *),
                 PP_FRAG3, PP_MEM_CATEGORY_CONFIG);

    SnortPreprocFree(config, sizeof(Frag3Config), PP_FRAG3, PP_MEM_CATEGORY_CONFIG);
}

/**
 * CleanExit func required by preprocessors
 */
static void Frag3CleanExit(int signal, void *foo)
{
    Frag3Frag *tmp;
    Frag3Config *pDefaultPolicyConfig = NULL;

    sfxhash_delete(f_cache);
    f_cache = NULL;

    pDefaultPolicyConfig = (Frag3Config *)sfPolicyUserDataGetDefault(frag3_config);

    /* Cleanup the preallocated frag nodes */
    if(pDefaultPolicyConfig->use_prealloc)
    {
        tmp = Frag3PreallocPop();
        while (tmp)
        {
            SnortPreprocFree(tmp->fptr, pkt_snaplen * sizeof(uint8_t),
                      PP_FRAG3, PP_MEM_CATEGORY_CONFIG);
            SnortPreprocFree(tmp, sizeof(Frag3Frag), 
                      PP_FRAG3, PP_MEM_CATEGORY_CONFIG);
            tmp = Frag3PreallocPop();
        }
    }

    Frag3FreeConfigs(frag3_config);

    Encode_Delete(defrag_pkt);
    defrag_pkt = NULL;

#ifdef GRE
    Encode_Delete(encap_defrag_pkt);
    encap_defrag_pkt = NULL;
#endif
}

static void Frag3Reset(int signal, void *foo)
{
    if (f_cache != NULL)
        sfxhash_make_empty(f_cache);
}

static void Frag3ResetStats(int signal, void *foo)
{
    memset(&f3stats, 0, sizeof(f3stats));
}


/**
 * Get a node from the prealloc_list
 *
 * @return pointer to a Frag3Frag preallocated structure or NULL if the list
 * is empty
 */
static inline Frag3Frag *Frag3PreallocPop(void)
{
    Frag3Frag *node;

    if(prealloc_frag_list)
    {
        node = prealloc_frag_list;
        prealloc_frag_list = prealloc_frag_list->next;
        if (prealloc_frag_list)
        {
            prealloc_frag_list->prev = NULL;
        }
        else
        {
            DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                        "Using last prealloc frag node\n"););
        }
        node->next = NULL;
        node->prev = NULL;
        node->offset = 0;
        node->size = 0;
        node->flen = 0;
        node->last = 0;
    }
    else
    {
        return NULL;
    }

    if (!node->fptr)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                        "Frag3Frag fptr is NULL!\n"););
    }

    prealloc_nodes_in_use++;
    return node;
}

/**
 * Put a prealloc node back into the prealloc_cache pool
 *
 * @param node Prealloc node to place back in the pool
 *
 * @return none
 */
static inline void Frag3PreallocPush(Frag3Frag *node)
{
    if (!prealloc_frag_list)
    {
        node->next = NULL;
        node->prev = NULL;
    }
    else
    {
        node->next = prealloc_frag_list;
        node->prev = NULL;
        prealloc_frag_list->prev = node;
    }

    prealloc_frag_list = node;
    node->data = NULL;
    if (!node->fptr)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FRAG,
                        "Frag3Frag fptr is NULL!\n"););
    }

    prealloc_nodes_in_use--;
    return;
}

/**
 * Plug a Frag3Frag into the fraglist of a FragTracker
 *
 * @param ft FragTracker to put the new node into
 * @param prev ptr to preceeding Frag3Frag in fraglist
 * @param next ptr to following Frag3Frag in fraglist
 * @param node ptr to node to put in list
 *
 * @return none
 */
static inline void Frag3FraglistAddNode(FragTracker *ft, Frag3Frag *prev,
        Frag3Frag *node)
{
    if(prev)
    {
        node->next = prev->next;
        node->prev = prev;
        prev->next = node;
        if (node->next)
            node->next->prev = node;
        else
            ft->fraglist_tail = node;
    }
    else
    {
        node->next = ft->fraglist;
        if (node->next)
            node->next->prev = node;
        else
            ft->fraglist_tail = node;
        ft->fraglist = node;
    }

    ft->fraglist_count++;
    return;
}

/**
 * Delete a Frag3Frag from a fraglist
 *
 * @param ft FragTracker to delete the frag from
 * @param node node to be deleted
 *
 * @return none
 */
static inline void Frag3FraglistDeleteNode(FragTracker *ft, Frag3Frag *node)
{
    DEBUG_WRAP(DebugMessage(DEBUG_FRAG, "Deleting list node %p (p %p n %p)\n",
                node, node->prev, node->next););

    if(node->prev)
    {
        node->prev->next = node->next;
    }
    else
    {
        ft->fraglist = node->next;
    }

    if(node->next)
    {
        node->next->prev = node->prev;
    }
    else
    {
        ft->fraglist_tail = node->prev;
    }

    Frag3DeleteFrag(node);
    ft->fraglist_count--;
}

/*
**
**  NAME
**    fpAddFragAlert::
**
**  DESCRIPTION
**    This function flags an alert per frag tracker.
**
**  FORMAL INPUTS
**    Packet *      - the packet to inspect
**    OptTreeNode * - the rule that generated the alert
**
**  FORMAL OUTPUTS
**    int - 0 if not flagged
**          1 if flagged
**
*/
int fpAddFragAlert(Packet *p, OptTreeNode *otn)
{
    FragTracker *ft = p->fragtracker;

    if ( !ft )
        return 0;

    if ( !otn )
        return 0;

    /* Only track a certain number of alerts per session */
    if ( ft->alert_count >= MAX_FRAG_ALERTS )
        return 0;

    ft->alert_gid[ft->alert_count] = otn->sigInfo.generator;
    ft->alert_sid[ft->alert_count] = otn->sigInfo.id;
    ft->alert_count++;

    return 1;
}

/*
**
**  NAME
**    fpFragAlerted::
**
**  DESCRIPTION
**    This function indicates whether or not an alert has been generated previously
**    in this session, but only if this is a rebuilt packet.
**
**  FORMAL INPUTS
**    Packet *      - the packet to inspect
**    OptTreeNode * - the rule that generated the alert
**
**  FORMAL OUTPUTS
**    int - 0 if alert NOT previously generated
**          1 if alert previously generated
**
*/
int fpFragAlerted(Packet *p, OptTreeNode *otn)
{
    FragTracker *ft = p->fragtracker;
    SigInfo *si = &otn->sigInfo;
    int      i;

    if ( !ft )
        return 0;

    for ( i = 0; i < ft->alert_count; i++ )
    {
        /*  If this is a rebuilt packet and we've seen this alert before, return
         *  that we have previously alerted on a non-rebuilt packet.
         */
        if ( (p->packet_flags & PKT_REBUILT_FRAG)
                && ft->alert_gid[i] == si->generator && ft->alert_sid[i] == si->id )
        {
            return 1;
        }
    }

    return 0;
}

#ifdef TARGET_BASED
int fragGetApplicationProtocolId(Packet *p)
{
    FragTracker *ft;
    /* Not caching this host_entry in the frag tracker so we can
     * swap the table out after processing this packet if we need
     * to.  */
    HostAttributeEntry *host_entry = NULL;
    uint16_t src_port = 0;
    uint16_t dst_port = 0;
    if (!p || !p->fragtracker)
    {
        return 0;
    }

    /* Must be a rebuilt frag... */
    if (!(p->packet_flags & PKT_REBUILT_FRAG))
    {
        return 0;
    }

    ft = (FragTracker *)p->fragtracker;

    if (ft->application_protocol != 0)
    {
        return ft->application_protocol;
    }

    switch (GET_IPH_PROTO(p))
    {
        case IPPROTO_TCP:
            ft->ipprotocol = protocolReferenceTCP;
            src_port = p->sp;
            dst_port = p->dp;
            break;
        case IPPROTO_UDP:
            ft->ipprotocol = protocolReferenceUDP;
            src_port = p->sp;
            dst_port = p->dp;
            break;
        case IPPROTO_ICMP:
            ft->ipprotocol = protocolReferenceICMP;
            break;
    }

    host_entry = SFAT_LookupHostEntryBySrc(p);
    if (host_entry)
    {
        ft->application_protocol = getApplicationProtocolId(host_entry,
                                    ft->ipprotocol,
                                    src_port,
                                    SFAT_SERVICE);
        if (ft->application_protocol != 0)
        {
            return ft->application_protocol;
        }
    }

    host_entry = SFAT_LookupHostEntryByDst(p);
    if (host_entry)
    {
        ft->application_protocol = getApplicationProtocolId(host_entry,
                                    ft->ipprotocol,
                                    dst_port,
                                    SFAT_SERVICE);
        if (ft->application_protocol != 0)
        {
            return ft->application_protocol;
        }
    }

    return ft->application_protocol;
}
#endif


#ifdef SNORT_RELOAD
static void Frag3ReloadGlobal(struct _SnortConfig *sc, char *args, void **new_config)
{
    tSfPolicyUserContextId frag3_swap_config = (tSfPolicyUserContextId)*new_config;
    Frag3Config *pDefaultPolicyConfig = NULL;
    Frag3Config *pCurrentPolicyConfig = NULL;
    tSfPolicyId policy_id = getParserPolicy(sc);

    if (!frag3_swap_config)
    {
        frag3_swap_config = sfPolicyConfigCreate();
        *new_config = (void *)frag3_swap_config;
    }

    sfPolicyUserPolicySet (frag3_swap_config, policy_id);
    pDefaultPolicyConfig = (Frag3Config *)sfPolicyUserDataGetDefault(frag3_swap_config);
    pCurrentPolicyConfig = (Frag3Config *)sfPolicyUserDataGetCurrent(frag3_swap_config);

    if ((policy_id != getDefaultPolicy()) && (pDefaultPolicyConfig == NULL))
    {
        ParseError("Frag3: Must configure default policy if other policies "
                   "are going to be used.\n");
    }

    if (pCurrentPolicyConfig != NULL)
    {
        FatalError("%s(%d) The frag3 global configuration can only be "
                   "configured once.\n", file_name, file_line);
    }

    pCurrentPolicyConfig = (Frag3Config *)SnortPreprocAlloc(1, sizeof(Frag3Config),
                                     PP_FRAG3, PP_MEM_CATEGORY_CONFIG);
    sfPolicyUserDataSetCurrent(frag3_swap_config, pCurrentPolicyConfig);

    /* setup default values */
    pCurrentPolicyConfig->max_frags = DEFAULT_MAX_FRAGS;
    pCurrentPolicyConfig->memcap = FRAG_MEMCAP;
    pCurrentPolicyConfig->static_frags = 0;
    pCurrentPolicyConfig->use_prealloc = 0;

    Frag3ParseGlobalArgs(pCurrentPolicyConfig, args);

    if (policy_id != getDefaultPolicy())
    {
        /* Can't set these in alternate policies */
        pCurrentPolicyConfig->memcap = pDefaultPolicyConfig->memcap;
        pCurrentPolicyConfig->max_frags = pDefaultPolicyConfig->max_frags;
        pCurrentPolicyConfig->use_prealloc = pDefaultPolicyConfig->use_prealloc;
        pCurrentPolicyConfig->static_frags = pDefaultPolicyConfig->static_frags;
    }
    else if (pCurrentPolicyConfig->use_prealloc &&
             (pCurrentPolicyConfig->static_frags == 0))
    {
        pCurrentPolicyConfig->static_frags = (uint32_t)pCurrentPolicyConfig->memcap /
            (sizeof(Frag3Frag) + sizeof(uint8_t) * pkt_snaplen) + 1;

        pCurrentPolicyConfig->ten_percent = pCurrentPolicyConfig->static_frags >> 5;
#ifdef REG_TEST
        if (REG_TEST_FLAG_RELOAD & getRegTestFlags())
        {
            printf("static frags is zero and memcap     : %lu\n",pCurrentPolicyConfig->memcap);
            printf("updated static_frags count          : %u\n",pCurrentPolicyConfig->static_frags);
        }
#endif
    }

    Frag3PrintGlobalConfig(pCurrentPolicyConfig);

    if ( !pCurrentPolicyConfig->disabled )
    {
        AddFuncToPreprocList(sc, Frag3Defrag, PP_FRAG3_PRIORITY, PP_FRAG3, PROTO_BIT__IP);
        session_api->enable_preproc_all_ports( sc, PP_FRAG3, PROTO_BIT__IP );
    }
}

static void Frag3ReloadEngine(struct _SnortConfig *sc, char *args, void **new_config)
{
    tSfPolicyUserContextId frag3_swap_config;
    Frag3Context *context;      /* context pointer */
    tSfPolicyId policy_id = getParserPolicy(sc);
    Frag3Config *config = NULL;

    frag3_swap_config = (tSfPolicyUserContextId)GetRelatedReloadData(sc, "frag3_global");
    config = (Frag3Config *)sfPolicyUserDataGet(frag3_swap_config, policy_id);
    if (config == NULL)
    {
        FatalError("[!] Unable to configure frag3 engine!\n"
                   "Frag3 global config has not been established, "
                   "please issue a \"preprocessor frag3_global\" directive\n");
    }

    context = (Frag3Context *) SnortPreprocAlloc(1, sizeof(Frag3Context), PP_FRAG3,
                                 PP_MEM_CATEGORY_CONFIG);

    context->frag_policy = FRAG_POLICY_DEFAULT;
    context->frag_timeout = FRAG_PRUNE_QUANTA; /* 60 seconds */
    context->min_ttl = FRAG3_MIN_TTL;
    context->frag3_alerts = 0;

    Frag3ParseArgs(sc, args, context);

    if (context->bound_addrs == NULL)
    {
        if (config->default_context != NULL)
            FatalError("Frag3 => only one non-bound context can be specified.\n");

        config->default_context = context;
    }

    if (config->frag3ContextList == NULL)
    {
        config->numFrag3Contexts = 1;
        config->frag3ContextList =
            (Frag3Context **)SnortPreprocAlloc(1, sizeof (Frag3Context *),
                          PP_FRAG3, PP_MEM_CATEGORY_CONFIG);
    }
    else
    {
        Frag3Context **tmpContextList;

        config->numFrag3Contexts++;
        tmpContextList = (Frag3Context **)
            SnortPreprocAlloc(config->numFrag3Contexts, sizeof (Frag3Context *),
                              PP_FRAG3, PP_MEM_CATEGORY_CONFIG);

        memcpy(tmpContextList, config->frag3ContextList,
               sizeof(Frag3Context *) * (config->numFrag3Contexts - 1));

        SnortPreprocFree(config->frag3ContextList, 
                 (config->numFrag3Contexts - 1) * sizeof (Frag3Context *),
                  PP_FRAG3, PP_MEM_CATEGORY_CONFIG);
        config->frag3ContextList = tmpContextList;
    }

    config->frag3ContextList[config->numFrag3Contexts - 1] = context;

    Frag3PrintEngineConfig(context);
}

static int Frag3ReloadVerifyPolicy(
        struct _SnortConfig *sc,
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    Frag3Config *pPolicyConfig = (Frag3Config *)pData;
    if ( pPolicyConfig->disabled )
        return 0;

    //do any housekeeping before freeing Frag3Config
    if ((policyId != getDefaultPolicy())
           && (pPolicyConfig->numFrag3Contexts == 0))
    {
        WarningMessage("Frag3VerifyConfig() policy engine required "
                "but not configured.\n");
        return -1;
    }

    return 0;
}

static uint32_t Frag3MemReloadAdjust(unsigned maxWork)
{
     Frag3Frag *tmp;
     SFXHASH_NODE *hnode;
#ifdef REG_TEST
     static uint32_t addstaticfrag_cnt, delstaticfrag_cnt;
#endif

     Frag3Config *newConfig = (Frag3Config *)sfPolicyUserDataGetCurrent(frag3_config);
     pkt_snaplen = DAQ_GetSnapLen();

     if(newConfig->static_frags == old_static_frags)
	 return maxWork;

     else if(newConfig->static_frags > old_static_frags)
     {
	if(newConfig->use_prealloc)
	{
	   for(;maxWork && (newConfig->static_frags > old_static_frags); maxWork--,old_static_frags++)
	   {
                tmp = (Frag3Frag *) SnortPreprocAlloc(1, sizeof(Frag3Frag), PP_FRAG3,
                                        PP_MEM_CATEGORY_CONFIG);
                tmp->fptr = (uint8_t *) SnortPreprocAlloc(pkt_snaplen, sizeof(uint8_t),
                                      PP_FRAG3, PP_MEM_CATEGORY_CONFIG);
                Frag3PreallocPush(tmp);
		prealloc_nodes_in_use++;
#ifdef REG_TEST
		addstaticfrag_cnt++;
#endif
	   }
	}
     }
     else
     {
        for(;maxWork && (newConfig->static_frags < old_static_frags); maxWork--,old_static_frags--)
        {
	   tmp = Frag3PreallocPop();
           if (tmp)
           {
                SnortPreprocFree(tmp->fptr, pkt_snaplen * sizeof(uint8_t),
                           PP_FRAG3, PP_MEM_CATEGORY_CONFIG);
                SnortPreprocFree(tmp, sizeof(Frag3Frag), PP_FRAG3,
                           PP_MEM_CATEGORY_CONFIG);
	        prealloc_nodes_in_use--;
#ifdef REG_TEST
                delstaticfrag_cnt++;
#endif
          }
	  else
	  {
	    /*exhausted prealloc frags */
	    break;
	  }
	}
	if(maxWork && (newConfig->static_frags < old_static_frags))
	{
            for (;maxWork && (newConfig->static_frags < old_static_frags); maxWork--,old_static_frags--)
            {
                 hnode = sfxhash_lru_node(f_cache);
                if (hnode)
                {
                        Frag3RemoveTracker(hnode->key, hnode->data);
			tmp = Frag3PreallocPop();
		        if (tmp)
	                {
		             SnortPreprocFree(tmp->fptr, pkt_snaplen * sizeof(uint8_t),
                                     PP_FRAG3, PP_MEM_CATEGORY_CONFIG);
		             SnortPreprocFree(tmp, sizeof(Frag3Frag), PP_FRAG3,
                                     PP_MEM_CATEGORY_CONFIG);
			     prealloc_nodes_in_use--;
		        }
#ifdef REG_TEST
		        delstaticfrag_cnt++;
#endif
                 }
            }
	}
     }
#ifdef REG_TEST
        if (REG_TEST_FLAG_RELOAD & getRegTestFlags())
        {
            if(newConfig->static_frags == old_static_frags)
	    {
		if(addstaticfrag_cnt)
			printf("Total prealloc frag nodes added    : %u\n",addstaticfrag_cnt);
		if(delstaticfrag_cnt)
			printf("Total prealloc frag nodes deleted  : %u\n",delstaticfrag_cnt);
	    }
        }
#endif
     return maxWork;
}

static bool Frag3ReloadAdjust(bool idle, tSfPolicyId raPolicyId, void* userData)
{
   unsigned initialMaxWork = idle ? 2048 : 5;
   unsigned maxWork;
   int iRet = -1;

   maxWork = Frag3MemReloadAdjust(initialMaxWork);

   if(maxWork)
   {
     iRet = sfxhash_change_memcap(f_cache, fcache_new_memcap, &maxWork);

#ifdef REG_TEST
     if(iRet == SFXHASH_OK && maxWork)
     {
       if (REG_TEST_FLAG_RELOAD & getRegTestFlags())
       {
          printf("Successfully updated Frag cache memcap :%lu\n",f_cache->mc.memcap);
       }
     }
#endif
   }
   return (maxWork != 0) ? true : false;
}


static int Frag3ReloadVerify(struct _SnortConfig *sc, void *swap_config)
{
    int rval;
    tSfPolicyUserContextId frag3_swap_config = (tSfPolicyUserContextId)swap_config;
    Frag3Config *pCurrDefaultPolicyConfig = NULL;
    Frag3Config *pSwapDefaultPolicyConfig = NULL;
    tSfPolicyId policy_id = 0;

    pCurrDefaultPolicyConfig = (Frag3Config *)sfPolicyUserDataGetDefault(frag3_config);
    pSwapDefaultPolicyConfig = (Frag3Config *)sfPolicyUserDataGetDefault(frag3_swap_config);

    if ((frag3_swap_config == NULL) || (frag3_config == NULL))
        return 0;

    if ((rval = sfPolicyUserDataIterate (sc, frag3_swap_config, Frag3ReloadVerifyPolicy)))
        return rval;

    policy_id = getParserPolicy(sc);
    if ((pSwapDefaultPolicyConfig->static_frags != pCurrDefaultPolicyConfig->static_frags) ||
	(pSwapDefaultPolicyConfig->max_frags != pCurrDefaultPolicyConfig->max_frags))
    {
	unsigned long max_frag_mem, table_mem;

        old_static_frags = pCurrDefaultPolicyConfig->static_frags;

        max_frag_mem = pSwapDefaultPolicyConfig->max_frags * (
               sizeof(FragTracker) +
               sizeof(SFXHASH_NODE) +
               sizeof (FRAGKEY) +
               sizeof(SFXHASH_NODE *));
        table_mem = (hashTableSize + 1) * sizeof(SFXHASH_NODE *);
        fcache_new_memcap = max_frag_mem + table_mem;

#ifdef REG_TEST
          if (REG_TEST_FLAG_RELOAD & getRegTestFlags())
          {
             printf("prealloc static frags old conf : %d new conf : %d\n",old_static_frags,pSwapDefaultPolicyConfig->static_frags);
             if (pSwapDefaultPolicyConfig->use_prealloc)
                printf("use_prealloc is enabled!\n");
             else
               printf("use_prealloc is disabled!\n");
	     printf("Frag cache new memcap value: %lu\n",fcache_new_memcap);
          }
#endif
          ReloadAdjustRegister(sc, "Frag3Reload", policy_id, &Frag3ReloadAdjust, NULL, NULL);
    }

    return 0;
}

static int Frag3ReloadSwapPolicy(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    Frag3Config *pPolicyConfig = (Frag3Config *)pData;

    //do any housekeeping before freeing Frag3Config
    if (pPolicyConfig->ref_count == 0)
    {
        sfPolicyUserDataClear (config, policyId);
        Frag3FreeConfig(pPolicyConfig);
    }

    return 0;
}

static void * Frag3ReloadSwap(struct _SnortConfig *sc, void *swap_config)
{
    tSfPolicyUserContextId frag3_swap_config = (tSfPolicyUserContextId)swap_config;
    tSfPolicyUserContextId old_config = frag3_config;

    if (frag3_swap_config == NULL)
        return NULL;

    frag3_config = frag3_swap_config;

    sfPolicyUserDataFreeIterate (old_config, Frag3ReloadSwapPolicy);

    if (sfPolicyUserPolicyGetActive(old_config) == 0)
        return (void *)old_config;

    return NULL;
}

static void Frag3ReloadSwapFree(void *data)
{
    if (data == NULL)
        return;

    Frag3FreeConfigs((tSfPolicyUserContextId)data);
}
#endif
