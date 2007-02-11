/*
 *  Template MIB group interface - disk.h
 *
 */
#ifndef _MIBGROUP_DISK_H
#define _MIBGROUP_DISK_H

void            init_disk(void);

config_require(util_funcs)

     extern FindVarMethod var_extensible_disk;

#include "mibdefs.h"

#define DISKDEVICE 3
#define DISKMINIMUM 4
#define DISKMINPERCENT 5
#define DISKTOTAL 6
#define DISKAVAIL 7
#define DISKUSED 8
#define DISKPERCENT 9
#define DISKPERCENTNODE 10

#endif                          /* _MIBGROUP_DISK_H */
