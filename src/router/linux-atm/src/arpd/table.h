/* table.h - ATMARP table */
 
/* Written 1995-1999 by Werner Almesberger, EPFL-LRC/ICA */
 

#ifndef TABLE_H
#define TABLE_H

#include <stdint.h>
#include <stdio.h>
#include <linux/atm.h>

#include "atmd.h"


typedef struct _vcc {
    int connecting;
    int active; /* indicate direction - for user entertainment only */
    int fd;
    struct _entry *entry;
    struct _vcc *prev,*next;
} VCC;

typedef enum {
    as_none,		/* invalid */
    as_resolv,		/* waiting for resolver response */
    as_invalid,		/* invalid, waiting for all VCs to be closed */
    as_valid,		/* valid */
} ADDR_STATE;

typedef struct _notify {
    UN_CTX ctx;		/* peer to send reply to */
    struct _notify *next;
} NOTIFY;

typedef struct _entry {
    ADDR_STATE state;
    int svc;
    uint32_t ip;
    struct sockaddr_atmsvc *addr; /* NULL if none */
    struct atm_qos qos;
    int sndbuf;
    int flags;
    TIMER *timer; /* currently active timer or NULL */
    int timeout; /* current interval - only necessary if using retries */
    int retries;
    VCC *vccs;
    NOTIFY *notify;
    struct _itf *itf;
    struct _entry *prev,*next; /* undefined if itf == NULL */
} ENTRY;

typedef struct _itf {
    uint32_t local_ip; /* @@@ */
    uint32_t netmask;
    int number;
    int mtu;
    struct atm_qos qos; /* default QOS */
    int sndbuf; /* default send buffer */
    ENTRY *table;
    ENTRY *arp_srv; /* NULL is none */
    struct _itf *prev,*next;
} ITF;


/*
 * May want to consider using one big unified table instead of lots of small
 * tables (one per interface).
 */


extern const char *entry_state_name[];
extern const char *vcc_state_name[];

extern ITF *itfs;
extern ENTRY *unknown_incoming;
extern VCC *unidirectional_vccs;

extern int pretty,merge;


ENTRY *alloc_entry(int svc);

ENTRY *lookup_ip(const ITF *itf,uint32_t ip);
ENTRY *lookup_addr(const ITF *itf,const struct sockaddr_atmsvc *addr);
ENTRY *lookup_incoming(const struct sockaddr_atmsvc *addr);

void table_changed(void);
int table_update(void);

#endif
