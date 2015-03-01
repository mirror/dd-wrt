
#ifndef __UNIFIED_LOG_H__
#define __UNIFIED_LOG_H__

#include <sys/types.h>
#ifdef LINUX
#include <stdint.h>
#endif
#include "sf_types.h"

struct u2_timeval
{
    uint32_t tv_sec;
    uint32_t tv_usec;
};

/* Miscelaneous data structures */
typedef struct SnortEvent
{
    uint32_t sig_generator;  
    uint32_t sig_id;         
    uint32_t sig_rev;        
    uint32_t classification; 
    uint32_t priority;       
    uint32_t event_id;       
    uint32_t event_reference;
    struct u2_timeval ref_time;  
} SnortEvent;

#define SNORT_EVENT_LENGTH 36

typedef struct _SnortPktHeader
{
    struct u2_timeval ts;
    uint32_t caplen;
    uint32_t pktlen;
} SnortPktHeader;

#define SNORT_PKT_HEADER_LENGTH 16

/* Snort Unified Log Record API ***********************************************/
typedef struct _UnifiedLog
{
    SnortEvent event;
    uint32_t flags;
    SnortPktHeader pkth;
    uint8_t *packet;
} UnifiedLog;

/**
 * Free a Unified Log record 
 */
int UnifiedLog_Destroy(UnifiedLog *unified_log);

#endif /*__UNIFIED_LOG_H__ */
