/*
 *  memory quantity mib groups
 *
 */
#ifndef _MIBGROUP_MEMORY_H
#define _MIBGROUP_MEMORY_H

#include "mibdefs.h"

void            init_memory(void);

/*
 * config file parsing routines 
 */
void            memory_parse_config(const char *, char *);
void            memory_free_config(void);

#define MEMTOTALSWAP 3
#define MEMAVAILSWAP 4
#define MEMTOTALREAL 5
#define MEMAVAILREAL 6
#define MEMTOTALSWAPTXT 7
#define MEMUSEDSWAPTXT 8
#define MEMTOTALREALTXT 9
#define MEMUSEDREALTXT 10
#define MEMTOTALFREE 11
#define MEMSWAPMINIMUM 12
#define MEMSHARED 13
#define MEMBUFFER 14
#define MEMCACHED 15
#define MEMSWAPERROR 16

#endif                          /* _MIBGROUP_MEMORY_H */
