/*
** Copyright (C) 2010 Sourcefire, Inc.
** Author: Michael R. Altizer <maltizer@sourcefire.com>
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
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef _DAQ_COMMON_H
#define _DAQ_COMMON_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdint.h>
#include <unistd.h>
#ifndef WIN32
#include <sys/time.h>
#endif

#ifndef SO_PUBLIC
#if defined _WIN32 || defined __CYGWIN__
#  if defined DAQ_DLL
#    ifdef __GNUC__
#      define SO_PUBLIC __attribute__((dllexport))
#    else
#      define SO_PUBLIC __declspec(dllexport)
#    endif
#  else
#    ifdef __GNUC__
#      define SO_PUBLIC __attribute__((dllimport))
#    else
#      define SO_PUBLIC __declspec(dllimport)
#    endif
#  endif
#  define DLL_LOCAL
#else
#  ifdef HAVE_VISIBILITY
#    define SO_PUBLIC  __attribute__ ((visibility("default")))
#    define SO_PRIVATE __attribute__ ((visibility("hidden")))
#  else
#    define SO_PUBLIC
#    define SO_PRIVATE
#  endif
#endif
#endif

#ifdef _WIN32
# ifdef DAQ_DLL
#  define DAQ_LINKAGE SO_PUBLIC
# else
#  define DAQ_LINKAGE
# endif
#else
# define DAQ_LINKAGE SO_PUBLIC
#endif

#define DAQ_SUCCESS          0  /* Success! */
#define DAQ_ERROR           -1  /* Generic error */
#define DAQ_ERROR_NOMEM     -2  /* Out of memory error */
#define DAQ_ERROR_NODEV     -3  /* No such device error */
#define DAQ_ERROR_NOTSUP    -4  /* Functionality is unsupported error */
#define DAQ_ERROR_NOMOD     -5  /* No module specified error */
#define DAQ_ERROR_NOCTX     -6  /* No context specified error */
#define DAQ_ERROR_INVAL     -7  /* Invalid argument/request error */
#define DAQ_ERROR_EXISTS    -8  /* Argument or device already exists */
#define DAQ_READFILE_EOF    -42 /* Hit the end of the file being read! */

#define DAQ_PKT_FLAG_HW_TCP_CS_GOOD  0x1

/* The DAQ packet header structure passed to DAQ Analysis Functions.  This should NEVER be modified by user applications. */
typedef struct _daq_pkthdr
{
    struct timeval ts;      /* Timestamp */
    uint32_t caplen;        /* Length of the portion present */
    uint32_t pktlen;        /* Length of this packet (off wire) */
    int device_index;       /* Index of the receiving interface. */
    uint32_t flags;         /* Flags for the packet (DAQ_PKT_FLAG_*) */
} DAQ_PktHdr_t;

typedef enum {
    DAQ_VERDICT_PASS,       /* Pass the packet. */
    DAQ_VERDICT_BLOCK,       /* Block the packet. */
    DAQ_VERDICT_REPLACE,    /* Pass a packet that has been modified in-place. (No resizing allowed!) */
    DAQ_VERDICT_WHITELIST,  /* Pass the packet and fastpath all future packets in the same flow systemwide. */
    DAQ_VERDICT_BLACKLIST,  /* Block the packet and block all future packets in the same flow systemwide. */
    DAQ_VERDICT_IGNORE,     /* Pass the packet and fastpath all future packets in the same flow for this application. */
    MAX_DAQ_VERDICT
} DAQ_Verdict;

typedef DAQ_Verdict (*DAQ_Analysis_Func_t)(void *user, const DAQ_PktHdr_t *hdr, const uint8_t *data);

typedef enum {
    DAQ_MODE_PASSIVE,
    DAQ_MODE_INLINE,
    DAQ_MODE_READ_FILE,
    MAX_DAQ_MODE
} DAQ_Mode;

#define DAQ_CFG_PROMISC     0x01

typedef struct _daq_dict_entry DAQ_Dict;

typedef struct _daq_config
{
    char *name;         /* Name of the interface(s) or file to be opened */
    int snaplen;        /* Maximum packet capture length */
    unsigned timeout;   /* Read timeout for acquire loop in milliseconds (0 = unlimited) */
    DAQ_Mode mode;      /* Module mode (DAQ_MODE_*) */
    uint32_t flags;     /* Other configuration flags (DAQ_CFG_*) */
    DAQ_Dict *values;   /* Dictionary of arbitrary key[:value] string pairs. */
    char *extra;        /* Miscellaneous configuration data to be passed to the DAQ module */
} DAQ_Config_t;

typedef enum {
    DAQ_STATE_UNINITIALIZED,
    DAQ_STATE_INITIALIZED,
    DAQ_STATE_STARTED,
    DAQ_STATE_STOPPED,
    DAQ_STATE_UNKNOWN,
    MAX_DAQ_STATE
} DAQ_State;

typedef struct _daq_stats
{
    uint64_t hw_packets_received;           /* Packets received by the hardware */
    uint64_t hw_packets_dropped;            /* Packets dropped by the hardware */
    uint64_t packets_received;              /* Packets received by this instance */
    uint64_t packets_filtered;              /* Packets filtered by this instance's BPF */
    uint64_t packets_injected;              /* Packets injected by this instance */
    uint64_t verdicts[MAX_DAQ_VERDICT];     /* Counters of packets handled per-verdict. */
} DAQ_Stats_t;

/* DAQ module type flags */
#define DAQ_TYPE_FILE_CAPABLE   0x01    /* can read from a file */
#define DAQ_TYPE_INTF_CAPABLE   0x02    /* can open live interfaces */
#define DAQ_TYPE_INLINE_CAPABLE 0x04    /* can form an inline bridge */
#define DAQ_TYPE_MULTI_INSTANCE 0x08    /* can be instantiated multiple times */
#define DAQ_TYPE_NO_UNPRIV      0x10    /* can not run unprivileged */

/* DAQ module capability flags */
#define DAQ_CAPA_NONE           0x000   /* no capabilities */
#define DAQ_CAPA_BLOCK          0x001   /* can block packets */
#define DAQ_CAPA_REPLACE        0x002   /* can replace/modify packet data (up to the original data size) */
#define DAQ_CAPA_INJECT         0x004   /* can inject packets */
#define DAQ_CAPA_WHITELIST      0x008   /* can whitelist flows */
#define DAQ_CAPA_BLACKLIST      0x010   /* can blacklist flows */
#define DAQ_CAPA_UNPRIV_START   0x020   /* can call start() without root privileges */
#define DAQ_CAPA_BREAKLOOP      0x040   /* can call breakloop() to break acquisition loop */
#define DAQ_CAPA_BPF            0x080   /* can call set_filter() to establish a BPF */
#define DAQ_CAPA_DEVICE_INDEX   0x100   /* can consistently fill the device_index field in DAQ_PktHdr */
#define DAQ_CAPA_INJECT_RAW     0x200   /* injection of raw packets (no layer-2 headers) */

typedef struct _daq_module DAQ_Module_t;

#endif /* _DAQ_COMMON_H */
